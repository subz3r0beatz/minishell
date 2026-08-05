/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_loop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 23:05:28 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 03:25:47 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	process_input(t_minishell *shell, char *input)
{
	add_history(input);
	shell->tokens = lexer(input, shell->token_type_table);
	if (!shell->tokens)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return ;
	}
	shell->ast = parser(shell, shell->tokens);
	if (!shell->ast)
		return ;
	shell->exit_status = exec(shell, shell->ast);
	free_ast(shell->ast);
	shell->ast = NULL;
}

static int	handle_quotes(const char *input, char *quote_state)
{
	size_t	i;
	char	quote;

	i = 0;
	quote = 0;
	while (input && input[i])
	{
		if ((quote == 0 && input[i] == '\\' && input[i + 1])
			|| (quote == '"' && input[i] == '\\'
				&& input[i + 1] && ft_strchr("\\\n`$\"'", input[i + 1])))
			i++;
		else if (quote == 0 && (input[i] == '"' || input[i] == '\''))
			quote = input[i];
		else if (quote != 0 && input[i] == quote)
			quote = 0;
		i++;
	}
	*quote_state = quote;
	return (quote != 0);
}

char	*read_line_non_interactive(int fd)
{
	char	*line;
	char	c;
	size_t	i;
	int		bytes;

	line = malloc(sizeof(char) * 4096);
	if (!line)
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	if (!line)
		return (NULL);
	i = 0;
	bytes = read(fd, &c, 1);
	if (bytes <= 0)
	{
		free(line);
		return (NULL);
	}
	while (bytes > 0 && c != '\n' && i < 4095)
	{
		line[i++] = c;
		bytes = read(fd, &c, 1);
	}
	line[i] = '\0';
	return (line);
}

static char	*quote_state_error(t_minishell *shell, char *input, char quote)
{
	ft_putstr_fd("minishell: unexpected EOF while "
		"looking for matching `", STDERR_FILENO);
	ft_putchar_fd(quote, STDERR_FILENO);
	ft_putstr_fd("'\nsyntax error: unexpected end of file\n",
		STDERR_FILENO);
	shell->exit_status = 2;
	free(input);
	return (NULL);
}

static char	*read_complete_input(t_minishell *shell, char *raw_input)
{
	char	*input;
	char	*next;
	char	*tmp;
	char	quote;

	input = raw_input;
	while (input && handle_quotes(input, &quote))
	{
		if (isatty(STDIN_FILENO))
			next = readline("> ");
		else
			next = read_line_non_interactive(STDIN_FILENO);
		if (!next)
			return (quote_state_error(shell, input, quote));
		tmp = ft_strjoin_3(input, "\n", next);
		free(input);
		free(next);
		input = tmp;
	}
	if (!input)
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	return (input);
}

void	main_loop(t_minishell *shell)
{
	int		status;
	char	*prompt;

	while (1)
	{
		init_interactive_signals();
		if (isatty(STDIN_FILENO))
		{
			status = build_prompt(shell->env, &prompt);
			shell->input = readline(prompt);
			if (status != 2)
				free(prompt);
		}
		else
			shell->input = read_line_non_interactive(STDIN_FILENO);
		if (!shell->input && isatty(STDIN_FILENO))
			ft_putstr_fd("exit\n", STDERR_FILENO);
		if (!shell->input)
			break ;
		shell->input = read_complete_input(shell, shell->input);
		if (shell->input && *shell->input)
			process_input(shell, shell->input);
		free(shell->input);
		shell->input = NULL;
	}
}

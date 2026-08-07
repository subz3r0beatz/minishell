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
		return ;
	shell->ast = parser(shell, shell->tokens);
	if (!shell->ast)
		return ;
	shell->exit_status = exec(shell, shell->ast);
	free_ast(shell->ast);
	shell->ast = NULL;
}

static int	grow_buf(char **buf, size_t len, size_t *cap)
{
	char	*new_buf;
	size_t	i;

	*cap *= 2;
	new_buf = malloc(sizeof(char) * (*cap));
	if (!new_buf)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		free(*buf);
		*buf = NULL;
		return (0);
	}
	i = 0;
	while (i < len)
	{
		new_buf[i] = (*buf)[i];
		i++;
	}
	free(*buf);
	*buf = new_buf;
	return (1);
}

char	*read_line_non_interactive(int fd)
{
	char	*buf;
	char	c;
	size_t	i;
	size_t	cap;
	int		b;

	cap = 128;
	buf = malloc(sizeof(char) * cap);
	if (!buf)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	i = 0;
	while ((b = read(fd, &c, 1)) > 0 && c != '\n')
	{
		if (i + 1 >= cap && !grow_buf(&buf, i, &cap))
			return (NULL);
		buf[i++] = c;
	}
	if (i == 0 && b <= 0)
	{
		free(buf);
		return (NULL);
	}
	buf[i] = '\0';
	return (buf);
}

void	loop(t_minishell *shell)
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

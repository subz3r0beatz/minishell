/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   loop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 23:05:28 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/19 04:20:53 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	trim_trailing_newlines(char *str)
{
	int	i;

	if (!str)
		return ;
	i = ft_strlen(str) - 1;
	while (i >= 0 && str[i] == '\n')
	{
		str[i] = '\0';
		i--;
	}
}

static void	exec_input(t_minishell *shell)
{
	shell->tokens = lexer(shell->input, shell->token_type_table);
	if (shell->tokens)
	{
		shell->ast = parser(shell, shell->tokens);
		if (shell->ast)
		{
			free_tokens(shell->tokens);
			shell->tokens = NULL;
			shell->exit_status = exec(shell, shell->ast);
			if (shell->exit_status == 130)
				ft_putstr_fd("\n", STDERR_FILENO);
			else if (shell->exit_status == 131)
				ft_putstr_fd("Quit\n", STDERR_FILENO);
			free_ast(shell->ast);
			shell->ast = NULL;
		}
		else
			free_tokens(shell->tokens);
		shell->tokens = NULL;
	}
	if (isatty(STDIN_FILENO) && shell->history && shell->history[0])
	{
		trim_trailing_newlines(shell->history);
		add_history(shell->history);
	}
}

static void	process_input(t_minishell *shell)
{
	size_t	i;
	char	*tmp;

	i = 0;
	while (shell->input && shell->input[i] && ft_iswhite(shell->input[i]))
		i++;
	if (!shell->input || !shell->input[i])
		return ;
	tmp = ft_strtrim(shell->input, "\n");
	if (!tmp)
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	if (!tmp)
		return ;
	free(shell->input);
	shell->input = tmp;
	free(shell->history);
	if (isatty(STDIN_FILENO))
		shell->history = ft_strdup(shell->input);
	if (isatty(STDIN_FILENO) && !shell->history)
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	if (isatty(STDIN_FILENO) && !shell->history)
		return ;
	exec_input(shell);
}

static int	handle_input(t_minishell *shell, int malloc_error)
{
	char	*saved_input;

	if (!shell->input && malloc_error)
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	if (!shell->input && isatty(STDIN_FILENO) && !malloc_error)
		ft_putstr_fd("exit\n", STDERR_FILENO);
	if (!shell->input)
		return (1);
	saved_input = shell->input;
	shell->input_line = shell->input;
	while (shell->input_line && *shell->input_line)
	{
		shell->input = extract_line(&shell->input_line, 1, &malloc_error);
		if (!shell->input)
			break ;
		process_input(shell);
		free(shell->input);
	}
	shell->input = saved_input;
	return (0);
}

void	loop(t_minishell *shell)
{
	char	buffer[128];
	int		malloc_error;

	buffer[0] = 0;
	while (1)
	{
		free(shell->input);
		malloc_error = 0;
		init_interactive_signals();
		if (isatty(STDIN_FILENO))
		{
			build_prompt(shell);
			rl_event_hook = rl_signal_check;
			shell->input = readline("$ ");
			rl_event_hook = NULL;
			if (g_signal_status == 130)
				shell->exit_status = 130;
			if (g_signal_status == 130)
				continue ;
		}
		else
			shell->input = ft_gnl(STDIN_FILENO, buffer, 128, &malloc_error);
		if (handle_input(shell, malloc_error))
			break ;
	}
}

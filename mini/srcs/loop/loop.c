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
	size_t	i;

	if (!input)
		return ;
	i = 0;
	while (input[i] && ft_iswhite(input[i]))
		i++;
	if (!input[i])
		return ;
	add_history(input);
	shell->tokens = lexer(input, shell->token_type_table);
	if (!shell->tokens)
		return ;
	shell->ast = parser(shell, shell->tokens);
	if (!shell->ast)
		return ;
	if (collect_heredocs(shell, shell->ast) == 0)
	{
		shell->exit_status = exec(shell, shell->ast);
		if (shell->exit_status == 130)
			ft_putstr_fd("\n", STDERR_FILENO);
		else if (shell->exit_status == 131)
			ft_putstr_fd("Quit\n", STDERR_FILENO);
	}
	free_ast(shell->ast);
	shell->ast = NULL;
}

void	loop(t_minishell *shell)
{
	char	buffer[128];
	int		malloc_error;

	buffer[0] = 0;
	while (1)
	{
		malloc_error = 0;
		init_interactive_signals();
		if (isatty(STDIN_FILENO))
		{
			build_prompt(shell);
			rl_event_hook = rl_signal_check;
			shell->input = readline("$ ");
			rl_event_hook = NULL;
			if (g_signal_status == 130)
			{
				shell->exit_status = 130;
				free(shell->input);
				shell->input = NULL;
				continue ;
			}
		}
		else
		{
			malloc_error = 1;
			shell->input = ft_gnl(STDIN_FILENO, buffer, 128, &malloc_error);
		}
		if (!shell->input && malloc_error)
			ft_putstr_fd("minishell: malloc: cannot allocate memory\n", STDERR_FILENO);
		if (!shell->input && isatty(STDIN_FILENO))
			ft_putstr_fd("exit\n", STDERR_FILENO);
		if (!shell->input)
			break ;
		shell->input = read_open_quotes(shell, shell->input, buffer);
		if (!shell->input && !isatty(STDIN_FILENO))
			break ;
		process_input(shell, shell->input);
		free(shell->input);
		shell->input = NULL;
	}
}

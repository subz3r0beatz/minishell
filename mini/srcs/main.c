/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 17:46:29 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 23:05:25 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	init_minishell(t_minishell *shell, char **argv, char **envp)
{
	shell->env = NULL;
	shell->exported = NULL;
	shell->exported_count = 0;
	shell->input = NULL;
	shell->tokens = NULL;
	shell->ast = NULL;
	shell->syn_err = 0;
	shell->argv0 = argv[0];
	shell->pid = NULL;
	shell->last_pid = NULL;
	shell->exit_status = 0;
	shell->double_root = 0;	
	if (build_env(shell, envp))
		exit_shell(shell, 1);
	init_token_type_table(shell->token_type_table);
	init_exec_func_table(shell->exec_func_table);
	init_builtin_func_table(shell->builtin_func_table);
}

int	main(int argc, char **argv, char **envp)
{
	t_minishell	shell;

	if (argc != 1)
	{
		ft_putendl_fd("minishell: too many arguments", STDERR_FILENO);
		return (1);
	}
	init_minishell(&shell, argv, envp);
	loop(&shell);
	exit_shell(&shell, shell.exit_status);
}

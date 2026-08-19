/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_subshell.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 15:22:20 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/19 03:50:44 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	wait_exec(t_minishell *shell, pid_t pid)
{
	int	status;

	waitpid(pid, &status, 0);
	init_interactive_signals();
	if (WIFEXITED(status))
		shell->exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
	{
		shell->exit_status = 128 + WTERMSIG(status);
		if (WTERMSIG(status) == SIGINT)
			g_signal_status = 130;
	}
	return (shell->exit_status);
}

int	exec_subshell(t_minishell *shell, t_ast_node *node)
{
	pid_t	pid;

	init_ignore_signals(1);
	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
		return (1);
	}
	if (pid == 0)
	{
		shell->is_child = 1;
		init_ignore_signals(0);
		if (redirections(node->redir))
			exit_shell(shell, 1);
		exit_shell(shell, exec(shell, node->left));
	}
	return (wait_exec(shell, pid));
}

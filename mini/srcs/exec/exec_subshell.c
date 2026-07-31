/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_subshell.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 15:22:20 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/31 16:50:49 by fldumas-         ###   ########.fr       */
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
		if (shell->exit_status - 128 == SIGINT)
			ft_putstr_fd("\n", STDERR_FILENO);
		if (shell->exit_status - 128 == SIGQUIT)
			ft_putstr_fd("Quit\n", STDERR_FILENO);
	}
	return (shell->exit_status);
}

int	exec_subshell(t_minishell *shell, t_ast_node *node, char *argv0)
{
	pid_t	pid;

	init_ignore_signals();
	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
		shell->exit_status = 1;
		return (1);
	}
	if (pid == 0)
	{
		init_child_signals();
		if (apply_redirections(node->redir))
			exit_shell(shell, NULL, 0, 1);
		exit_shell(shell, NULL, 0, exec(shell, node->left, argv0));
	}
	return (wait_exec(shell, pid));
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 18:11:59 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/30 02:23:11 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	exec_pipe(t_minishell *shell, t_ast_node *node)
{
	int		pdf[2];
	pid_t	left_pid;
	pid_t	right_pid;
	int		status_left;
	int		status_right;

	if (pipe(pdf) < 0)
		return (1);
	init_ignore_signals();
	left_pid = fork();
	if (left_pid == 0)
	{
		init_child_signals();
		close(pdf[0]);
		dup2(pdf[1], STDOUT_FILENO);
		close(pdf[1]);
		exec(shell, node->left, NULL);
	}
	right_pid = fork();
	if (right_pid == 0)
	{
		init_child_signals();
		close(pfd[1]);
		dup2(pfd[0], STDIN_FILENO);
		close(pfd[0]);
		exec(shell, node->right, NULL);
	}
	close(pfd[0]);
	close(pfd[1]);
	waitpid(left_pid, &status_left, 0);
	waitpid(right_pid, &status_right, 0);
	if (WEXITSTATUS(status_left) != 0 || WEXITSTATUS(status_right) != 0)
	{
		g_exit_status = 1;
		return (1);
	}
	return (0);
}

int	exec(t_minishell *shell, t_ast_node *node, char *argv0)
{
	int	status;

	if (!node)
		return (0);
	return (shell->exec_func_table[node->type](shell, node, argv0));
}

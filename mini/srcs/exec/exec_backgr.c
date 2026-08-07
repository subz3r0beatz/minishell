/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_backgr.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 15:13:06 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:33:19 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	exec_backgr(t_minishell *shell, t_ast_node *node)
{
	pid_t	pid;

	if (!node)
		return (shell->exit_status);
	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
		shell->exit_status = 1;
		return (1);
	}
	if (pid == 0)
	{
		init_ignore_signals(0);
		exit_shell(shell, exec(shell, node->left));
	}
	waitpid(pid, NULL, WNOHANG);
	free(shell->last_pid);
	shell->last_pid = ft_itoa(pid);
	if (!shell->last_pid)
		ft_putstr_fd("minishell: exec: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	shell->exit_status = 0;
	if (node->right)
		return (exec(shell, node->right));
	return (0);
}

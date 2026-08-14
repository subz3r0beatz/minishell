/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmd.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 16:13:55 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 03:24:11 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	handle_empty(t_minishell *shell, t_ast_node *node)
{
	int	saved_stdin;
	int	saved_stdout;

	if (init_saved_std(shell, node->redir, &saved_stdin, &saved_stdout))
	{
		shell->exit_status = 1;
		return (shell->exit_status);
	}
	if (apply_redirections(shell, node->redir))
	{
		if (g_signal_status == 130)
			shell->exit_status = 130;
		else
			shell->exit_status = 1;
	}
	else
		shell->exit_status = 0;
	restore_fds(saved_stdin, saved_stdout);
	return (shell->exit_status);
}

int	exec_cmd(t_minishell *shell, t_ast_node *node)
{
	int	builtin;

	if (!node)
		return (shell->exit_status);
	if (expand(shell, node) != 0)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		shell->exit_status = 1;
		return (1);
	}
	if (!node->args || !node->args[0])
		return (handle_empty(shell, node));
	builtin = is_builtin(node->args[0]);
	if (builtin >= 3 && builtin <= 6)
		return (exec_builtin(shell, node, builtin));
	return (exec_binary(shell, node));
}

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

int	exec_cmd(t_minishell *shell, t_ast_node *node)
{
	int	builtin;

	if (!node)
		return (shell->exit_status);
	if (expand(shell, node) != 0)
	{
		ft_putstr_fd("minishell: malloc: cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	if (!node->args || !node->args[0])
		return (redirections(node->redir));
	builtin = is_builtin(node->args[0]);
	if (builtin >= 3 && builtin <= 6)
		return (exec_builtin(shell, node, builtin));
	return (exec_binary(shell, node));
}

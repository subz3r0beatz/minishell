/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_semi.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 13:17:56 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:18:37 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	exec_semi(t_minishell *shell, t_ast_node *node)
{
	if (!node)
		return (shell->exit_status);
	exec(shell, node->left);
	if (node->right)
		return (exec(shell, node->right));
	return (shell->exit_status);
}

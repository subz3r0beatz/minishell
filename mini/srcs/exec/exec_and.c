/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_and.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 13:30:48 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:33:30 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	exec_and(t_minishell *shell, t_ast_node *node)
{
	int	status;

	if (!node)
		return (shell->exit_status);
	status = exec(shell, node->left);
	if (status == 0)
		return (exec(shell, node->right));
	return (status);
}

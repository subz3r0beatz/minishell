/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_or.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 13:04:20 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:25:56 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	exec_or(t_minishell *shell, t_ast_node *node)
{
	int	status;

	status = exec(shell, node->left);
	if (status != 0 && g_signal_status != 130)
		status = exec(shell, node->right);
	return (status);
}

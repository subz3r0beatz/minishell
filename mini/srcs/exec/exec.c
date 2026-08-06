/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 18:11:59 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 14:44:55 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	exec(t_minishell *shell, t_ast_node *node)
{
	if (!node)
		return (shell->exit_status);
	return (shell->exec_func_table[node->type](shell, node));
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 18:11:59 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/31 16:48:22 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	exec(t_minishell *shell, t_ast_node *node, char *argv0)
{
	int	status;

	if (!node)
		return (0);
	return (shell->exec_func_table[node->type](shell, node, argv0));
}

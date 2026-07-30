/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_exec_func_table.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 02:09:12 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/30 02:15:03 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	init_exec_func_table(int (*exec_func_table[7])(t_minishell *shell,
				t_ast_node *node, char *argv0))
{
	exec_func_table[0] = exec_cmd;
	exec_func_table[1] = exec_pipe;
	exec_func_table[2] = exec_and;
	exec_func_table[3] = exec_or;
	exec_func_table[4] = exec_subshell;
	exec_func_table[5] = exec_semi;
	exec_func_table[6] = exec_backgr;
}

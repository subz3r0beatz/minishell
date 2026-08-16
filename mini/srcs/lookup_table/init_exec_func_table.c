/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_exec_func_table.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 02:09:12 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:31:22 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	init_exec_func_table(int (*exec_func_table[7])(t_minishell *shell,
				t_ast_node *node))
{
	exec_func_table[NODE_CMD] = exec_cmd;
	exec_func_table[NODE_PIPE] = exec_pipe;
	exec_func_table[NODE_AND] = exec_and;
	exec_func_table[NODE_OR] = exec_or;
	exec_func_table[NODE_SUBSHELL] = exec_subshell;
	exec_func_table[NODE_SEMI] = exec_semi;
	exec_func_table[NODE_BACKGR] = exec_backgr;
	exec_func_table[NODE_NEWLINE] = exec_newline;
}

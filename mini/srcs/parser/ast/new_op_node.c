/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_op_node.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 15:29:26 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/25 15:30:39 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast_node	*new_op_node(t_node_type type, t_ast_node *left, t_ast_node *right)
{
	t_ast_node	*node;

	node = malloc(sizeof(t_ast_node));
	if (!node)
		return (NULL);
	node->type = type;
	node->left = left;
	node->right = right;
	node->args = NULL;
	node->redir = NULL;
	return (node);
}

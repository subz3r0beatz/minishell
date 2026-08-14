/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_list.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 14:55:28 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 21:50:20 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_ast_node	*create_list_node(t_minishell *shell, t_token **token,
	t_ast_node *left, t_node_type op)
{
	t_ast_node	*right;
	t_ast_node	*parent;

	if (!*token || (*token)->type == TOKEN_RPAREN)
	{
		parent = new_op_node(op, left, NULL);
		if (!parent)
			return (free_ast(left));
		return (parent);
	}
	right = parse_logic(shell, token);
	if (!right)
		return (free_ast(left));
	parent = new_op_node(op, left, right);
	if (!parent)
	{
		free_ast(left);
		return (free_ast(right));
	}
	return (parent);
}

static t_ast_node	*loop_list(t_minishell *shell, t_token **token,
	t_ast_node *left)
{
	t_node_type	op;

	while (*token && ft_strlen((*token)->value) == 1
		&& ((*token)->type == TOKEN_SEMI || (*token)->type == TOKEN_BACKGR
		|| (*token)->type == TOKEN_NEWLINE))
	{
		op = NODE_SEMI;
		if ((*token)->type == TOKEN_BACKGR)
			op = NODE_BACKGR;
		*token = (*token)->next;
		left = create_list_node(shell, token, left, op);
		if (!left)
			return (NULL);
		if (!*token || (*token)->type == TOKEN_RPAREN)
			break ;
	}
	return (left);
}

t_ast_node	*parse_list(t_minishell *shell, t_token **token)
{
	t_ast_node	*left;

	while (*token && (*token)->type == TOKEN_NEWLINE)
		*token = (*token)->next;
	if (!*token || (*token)->type == TOKEN_RPAREN)
		return (NULL);
	left = parse_logic(shell, token);
	if (!left)
		return (NULL);
	left = loop_list(shell, token, left);
	return (left);
}

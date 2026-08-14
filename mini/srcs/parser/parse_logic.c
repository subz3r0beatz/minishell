/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_logic.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 17:19:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 19:23:54 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_ast_node	*loop_logic(t_minishell *shell, t_token **token,
	t_ast_node *left)
{
	t_ast_node	*right;
	t_ast_node	*parent;
	t_node_type	op;

	while (*token
		&& ((*token)->type == TOKEN_AND || (*token)->type == TOKEN_OR))
	{
		op = NODE_OR;
		if ((*token)->type == TOKEN_AND)
			op = NODE_AND;
		*token = (*token)->next;
		while (*token && (*token)->type == TOKEN_NEWLINE)
			*token = (*token)->next;
		if (!*token && !syntax_error(shell, *token))
			return (free_ast(left));
		right = parse_pipeline(shell, token);
		if (!right)
			return (free_ast(left));
		parent = new_op_node(op, left, right);
		if (!parent && !free_ast(left))
			return (free_ast(right));
		left = parent;
	}
	return (left);
}

t_ast_node	*parse_logic(t_minishell *shell, t_token **token)
{
	t_ast_node	*left;

	left = parse_pipeline(shell, token);
	if (!left)
		return (NULL);
	left = loop_logic(shell, token, left);
	return (left);
}

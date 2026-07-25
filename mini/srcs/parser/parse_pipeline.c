/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_pipeline.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 16:19:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/25 17:31:01 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast_node	*parse_pipeline(t_token **token)
{
	t_ast_node	*left;
	t_ast_node	*right;
	t_ast_node	*parent;

	left = parse_cmd(token);
	if (!left)
		return (NULL);
	while (*token && (*token)->type == TOKEN_PIPE)
	{
		*token = (*token)->next;
		if (!*token)
			return (free_ast(left));
		right = parse_cmd(token);
		if (!right)
			return (free_ast(left));
		parent = new_op_node(NODE_PIPE, left, right);
		if (!parent)
		{
			free_ast(left);
			return (free_ast(right));
		}
		left = parent;
	}
	return (left);
}

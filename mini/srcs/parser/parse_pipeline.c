/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_pipeline.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 16:19:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 19:24:09 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast_node	*parse_pipeline(t_minishell *shell, t_token **token)
{
	t_ast_node	*left;
	t_ast_node	*right;
	t_ast_node	*parent;

	left = parse_cmd(shell, token);
	if (!left)
		return (NULL);
	while (*token && (*token)->type == TOKEN_PIPE)
	{
		*token = (*token)->next;
		while (*token && (*token)->type == TOKEN_NEWLINE)
			*token = (*token)->next;
		if (!*token && !syntax_error(shell, *token))
			return (free_ast(left));
		right = parse_cmd(shell, token);
		if (!right)
			return (free_ast(left));
		parent = new_op_node(NODE_PIPE, left, right);
		if (!parent && !free_ast(left))
			return (free_ast(right));
		left = parent;
	}
	return (left);
}

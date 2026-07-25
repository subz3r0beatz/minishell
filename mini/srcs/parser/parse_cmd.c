/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_cmd.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 15:32:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/25 17:39:49 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	**add_to_matrix(char **matrix, char *str)
{
	char	**new_matrix;
	size_t	len;

	len = ft_memlen(matrix, sizeof(char *));
	new_matrix = ft_realloc(matrix, len + 2, sizeof(char *));
	if (!new_matrix)
	{
		ft_free_matrix(matrix, len);
		return (NULL);
	}
	new_matrix[len] = ft_strdup(str);
	if (!new_matrix[len])
	{
		ft_free_matrix(new_matrix, len);
		return (NULL);
	}
	new_matrix[len + 1] = NULL;
	return (new_matrix);
}

static t_ast_node	*parse_subshell(t_token **token)
{
	t_ast_node	*sub_ast;
	t_ast_node	*node;

	*token = (*token)->next;
	sub_ast = parse_logic(token);
	if (!sub_ast)
		return (NULL);
	if (!*token || (*token)->type != TOKEN_RPAREN)
		return (free_ast(sub_ast));
	*token = (*token)->next;
	node = new_op_node(NODE_SUBSHELL, sub_ast, NULL);
	if (!node)
		return (free_ast(sub_ast));
	while (*token && ((*token)->type == TOKEN_LESS
			|| (*token)->type == TOKEN_GREAT || (*token)->type == TOKEN_DLESS
			|| (*token)->type == TOKEN_DGREAT))
	{
		if (parse_redir(token, &node->redir))
			return (free_ast(node));
	}
	return (node);
}

t_ast_node	*parse_cmd(t_token **token)
{
	t_ast_node	*node;

	if (!token || !*token)
		return (NULL);
	if ((*token)->type == TOKEN_LPAREN)
		return (parse_subshell(token));
	if (new_cmd_node(&node))
		return (NULL);
	while (*token && ((*token)->type == TOKEN_WORD
			|| (*token)->type == TOKEN_LESS || (*token)->type == TOKEN_GREAT
			|| (*token)->type == TOKEN_DLESS || (*token)->type == TOKEN_DGREAT))
	{
		if ((*token)->type == TOKEN_WORD)
		{
			node->args = add_to_matrix(node->args, (*token)->value);
			if (!node->args)
				return (free_ast(node));
			*token = (*token)->next;
		}
		else if (parse_redir(token, &node->redir))
			return (free_ast(node));
	}
	if (!node->args && !node->redir)
		return (free_ast(node));
	return (node);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_cmd.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 15:32:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/17 03:27:10 by fldumas-         ###   ########.fr       */
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

t_ast_node	*parse_cmd(t_token **token)
{
	t_ast_node	*node;

	if (!token || !*token)
		return (NULL);
	node = new_cmd_node();
	if (!node)
		return (NULL);
	while (*token && (*token)->type == TOKEN_WORD)
	{
		node->args = add_to_matrix(node->args, (*token)->value);
		if (!node->args)
			return (NULL);
		*token = (*token)->next;
	}
	return (node);
}

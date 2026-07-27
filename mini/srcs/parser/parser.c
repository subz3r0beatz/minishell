/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 17:48:19 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/26 14:55:51 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast_node	*parser(t_token *tokens)
{
	t_ast_node	*ast;
	t_token		*curr;

	if (!tokens)
		return (NULL);
	curr = tokens;
	ast = parse_list(&curr);
	if (curr != NULL)
		return (free_ast(ast));
	return (ast);
}

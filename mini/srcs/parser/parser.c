/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 17:48:19 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 12:16:59 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_ast_node	*parse_logic(t_token **token)
{
	t_ast_node		*left;
	t_ast_node		*right;
	t_token_type	op;

	left = parse_pipeline(token);
}

t_ast_node	*parser(t_token *head)
{

	return (NULL);
}

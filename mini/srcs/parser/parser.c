/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 17:48:19 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/18 00:44:08 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast_node	*parser(t_minishell *shell, t_token *tokens)
{
	t_ast_node	*ast;
	t_token		*curr;

	if (!tokens)
		return (NULL);
	curr = tokens;
	shell->syn_err = 0;
	while (curr && curr->type == TOKEN_NEWLINE)
		curr = curr->next;
	if (!curr)
		return (NULL);
	ast = parse_list(shell, &curr);
	if (shell->syn_err)
		return (free_ast(ast));
	while (curr && curr->type == TOKEN_NEWLINE)
		curr = curr->next;
	if (curr != NULL)
	{
		syntax_error(shell, curr);
		return (free_ast(ast));
	}
	return (ast);
}

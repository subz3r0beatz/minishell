/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 15:58:21 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/27 23:50:11 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	expand(t_minishell *shell, t_ast_node *node, char *argv0)
{
	size_t	i;

	if (!node)
		return (0);
	i = 0;
	while (node->args && node->args[i])
	{
		if (node->args[i][0] == '~'
			&& (!node->args[i][1] || node->args[i][1] == '/'))
			node->args[i] = expand_home(shell, node->args[i]);
		if (!node->args[i])
			return (1);
		node->args[i] = expand_word(shell, node->args[i], argv0);
		if (!node->args[i])
			return (1);
		node->args[i] = strip_quotes(node->args[i]);
		if (!node->args[i])
			return (1);
		i++;
	}
	return (expand_redirs(shell, node->redir, argv0));
}

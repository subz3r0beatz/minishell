/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_cmd_node.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 15:30:58 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 18:19:27 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	new_cmd_node(t_ast_node **node)
{
	*node = malloc(sizeof(t_ast_node));
	if (!*node)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	(*node)->type = NODE_CMD;
	(*node)->left = NULL;
	(*node)->right = NULL;
	(*node)->args = NULL;
	(*node)->redir = NULL;
	return (0);
}

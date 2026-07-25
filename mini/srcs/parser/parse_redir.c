/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_redir.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 16:15:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/25 17:31:09 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_redir	*new_redir_node(t_token_type type, char *file)
{
	t_redir	*redir;

	redir = malloc(sizeof(t_redir));
	if (!redir)
		return (NULL);
	redir->type = type;
	redir->file = ft_strdup(file);
	if (!redir->file)
	{
		free(redir);
		return (NULL);
	}
	redir->next = NULL;
	return (redir);
}

int	parse_redir(t_token **token, t_redir **redir_head)
{
	t_token_type	type;
	t_redir			*new_node;
	t_redir			*tmp;

	type = (*token)->type;
	*token = (*token)->next;
	if (!*token || (*token)->type != TOKEN_WORD)
		return (1);
	new_node = new_redir_node(type, (*token)->value);
	if (!new_node)
		return (1);
	if (!*redir_head)
		*redir_head = new_node;
	else
	{
		tmp = *redir_head;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = new_node;
	}
	*token = (*token)->next;
	return (0);
}

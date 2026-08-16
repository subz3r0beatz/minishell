/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_ast.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 16:12:29 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/25 17:33:31 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	free_redirs(t_redir *redir)
{
	t_redir	*tmp;

	while (redir)
	{
		tmp = redir;
		redir = redir->next;
		if (tmp->fd >= 0)
			close(tmp->fd);
		free(tmp->file);
		free(tmp);
	}
}

t_ast_node	*free_ast(t_ast_node *ast)
{
	size_t	i;

	if (!ast)
		return (NULL);
	free_ast(ast->left);
	free_ast(ast->right);
	if (ast->args)
	{
		i = 0;
		while (ast->args[i])
			free(ast->args[i++]);
		free(ast->args);
	}
	if (ast->redir)
		free_redirs(ast->redir);
	free(ast);
	return (NULL);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 18:58:01 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/27 23:49:58 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	expand_redirs(t_minishell *shell, t_redir *redir, char *argv0)
{
	while (redir)
	{
		if (redir->file)
		{
			if (redir->file[0] == '~'
				&& (!redir->file[1] || redir->file[1] == '/'))
				redir->file = expand_home(shell, redir->file);
			if (!redir->file)
				return (1);
			redir->file = expand_word(shell, redir->file, argv0);
			if (!redir->file)
				return (1);
			redir->file = strip_quotes(redir->file);
			if (!redir->file)
				return (1);
		}
		redir = redir->next;
	}
	return (0);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_redirs.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 18:58:01 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:15:28 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	expand_redirs(t_minishell *shell, t_redir *redir)
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
			redir->file = expand_word(shell, redir->file);
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

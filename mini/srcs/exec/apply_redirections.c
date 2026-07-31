/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   apply_redirections.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 16:14:38 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/31 17:05:16 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	open_error(char	*file)
{
	perror(file);
	return (1);
}

int	apply_redirections(t_redir *redir)
{
	int	fd;

	while (redir)
	{
		if (redir->type == TOKEN_LESS)
		{
			fd = open(redir->token, O_RDONLY);
			if (fd < 0)
				return (open_error(redir->file));
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		else if (redir->type == TOKEN_GREAT)
		{
			fd = open(redir->token, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd < 0)
				return (open_error(redir->file));
			dup2(fd, 1);
			close(fd);
		}
		else if (redir->type == TOKEN_DGREAT)
		{
			fd = open(redir->token, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (fd < 0)
				return (open_error(redir->file));
			dup2(fd, 1);
			close(fd);
		}
		redir = redir->next;
	}
	return (0);
}

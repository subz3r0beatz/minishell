/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   apply_redirections.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 16:14:38 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 03:22:54 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	fd_error_message(t_redir *redir)
{
	int	err;

	err = errno;
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	errno = err;
	perror(redir->file);
	return (1);
}

static int	update_fd(int *fd, int new_fd)
{
	if (new_fd < 0)
		return (1);
	if (*fd >= 0)
		close(*fd);
	*fd = new_fd;
	return (0);
}

static int	process_redir(t_redir *redir, int *last_in, int *last_out)
{
	int	fd;

	if (redir->type == TOKEN_DLESS)
	{
		fd = redir->fd;
		redir->fd = -1;
	}
	else if (redir->type == TOKEN_TLESS)
		fd = handle_herestring(redir->file);
	else if (redir->type == TOKEN_LESS)
		fd = open(redir->file, O_RDONLY);
	else if (redir->type == TOKEN_GREAT)
		fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else if (redir->type == TOKEN_DGREAT)
		fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (redir->type != TOKEN_DLESS && redir->type != TOKEN_TLESS && fd < 0)
		return (fd_error_message(redir));
	if (redir->type == TOKEN_GREAT || redir->type == TOKEN_DGREAT)
		return (update_fd(last_out, fd));
	return (update_fd(last_in, fd));
}

int	redirections(t_redir *redir)
{
	int	last_in;
	int	last_out;

	last_in = -1;
	last_out = -1;
	while (redir)
	{
		if (process_redir(redir, &last_in, &last_out))
		{
			if (last_in >= 0)
				close(last_in);
			if (last_out >= 0)
				close(last_out);
			return (1);
		}
		redir = redir->next;
	}
	return (restore_fds(last_in, last_out));
}

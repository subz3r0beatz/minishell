/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   apply_redirections.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 16:14:38 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 18:02:25 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	open_error(char	*file, int err)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	errno = err;
	perror(file);
	return (1);
}

int	handle_stdin(t_redir *redir, int fd)
{
	int	err;

	if (fd < 0)
	{
		err = errno;
		return (open_error(redir->file, err));
	}
	dup2(fd, STDIN_FILENO);
	close(fd);
	return (0);
}

int	handle_stdout(t_redir *redir, int fd)
{
	int	err;

	if (fd < 0)
	{
		err = errno;
		return (open_error(redir->file, err));
	}
	dup2(fd, STDOUT_FILENO);
	close(fd);
	return (0);
}

int	apply_redirections(t_redir *redir)
{
	int	fd;

	while (redir)
	{
		if (redir->type == TOKEN_LESS)
		{
			fd = open(redir->file, O_RDONLY);
			if (handle_stdin(redir, fd))
				return (1);
		}
		else if (redir->type == TOKEN_GREAT)
		{
			fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (handle_stdout(redir, fd))
				return (1);
		}
		else if (redir->type == TOKEN_DGREAT)
		{
			fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (handle_stdout(redir, fd))
				return (1);
		}
		redir = redir->next;
	}
	return (0);
}

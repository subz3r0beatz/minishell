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

int	restore_fds(int in, int out)
{
	if (in >= 0 && in != STDIN_FILENO)
	{
		if (dup2(in, STDIN_FILENO) < 0)
		{
			perror("minishell: dup2");
			close(in);
			if (out >= 0)
				close(out);
			return (1);
		}
		close(in);
	}
	if (out >= 0 && out != STDOUT_FILENO)
	{
		if (dup2(out, STDOUT_FILENO) < 0)
		{
			perror("minishell: dup2");
			close(out);
			return (1);
		}
		close(out);
	}
	return (0);
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

int	handle_herestring(char *file)
{
	int	pfd[2];

	if (pipe(pfd) < 0)
	{
		ft_putstr_fd("minishell: exec: pipe failed\n", STDERR_FILENO);
		return (-1);
	}
	if (file)
		ft_putstr_fd(file, pfd[1]);
	ft_putstr_fd("\n", pfd[1]);
	close(pfd[1]);
	return (pfd[0]);
}

static int	process_redir(t_redir *redir, int *last_in, int *last_out)
{
	int	fd;
	int	err;

	if (redir->type == TOKEN_DLESS)
		fd = redir->fd;
	else if (redir->type == TOKEN_TLESS)
		fd = handle_herestring(redir->file);
	else if (redir->type == TOKEN_LESS)
		fd = open(redir->file, O_RDONLY);
	else if (redir->type == TOKEN_GREAT)
		fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else if (redir->type == TOKEN_DGREAT)
		fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (redir->type != TOKEN_DLESS && redir->type != TOKEN_TLESS && fd < 0)
	{
		err = errno;
		ft_putstr_fd("minishell: ", STDERR_FILENO);
		errno = err;
		perror(redir->file);
		return (1);
	}
	if (redir->type == TOKEN_GREAT || redir->type == TOKEN_DGREAT)
		return (update_fd(last_out, fd));
	return (update_fd(last_in, fd));
}

int	apply_redirections(t_minishell *shell, t_redir *redir)
{
	int	last_in;
	int	last_out;

	(void)shell;
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

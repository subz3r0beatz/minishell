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

int	handle_here_string(char *file)
{
	int	pfd[2];

	if (pipe(pfd) < 0)
	{
		ft_putstr_fd("minishell: exec: pipe failed\n", STDERR_FILENO);
		return (1);
	}
	if (file)
		ft_putstr_fd(file, pfd[1]);
	ft_putstr_fd("\n", pfd[1]);
	close(pfd[1]);
	if (dup2(pfd[0], STDIN_FILENO) < 0)
	{
		close(pfd[0]);
		return (1);
	}
	close(pfd[0]);
	return (0);
}

int	apply_redirections(t_minishell *shell, t_redir *redir)
{
	int	fd;

	while (redir)
	{
		if (redir->type == TOKEN_LESS)
			fd = open(redir->file, O_RDONLY);
		else if (redir->type == TOKEN_GREAT)
			fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else if (redir->type == TOKEN_DGREAT)
			fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (redir->type == TOKEN_DLESS && handle_heredoc(shell, redir->file))
			return (1);
		else if (redir->type == TOKEN_LESS && handle_stdin(redir, fd))
			return (1);
		else if ((redir->type == TOKEN_GREAT || redir->type == TOKEN_DGREAT)
			&& handle_stdout(redir, fd))
			return (1);
		else if (redir->type == TOKEN_TLESS && handle_here_string(redir->file))
			return (1);
		redir = redir->next;
	}
	return (0);
}

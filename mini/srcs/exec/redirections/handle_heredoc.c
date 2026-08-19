/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_heredoc.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:45:59 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/19 04:22:00 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	clean_error(int pfd[2])
{
	close(pfd[0]);
	close(pfd[1]);
	ft_putstr_fd("minishell: malloc: cannot allocate memory\n", STDERR_FILENO);
	return (-1);
}

static int	heredoc_loop_error(char *clean, int pfd[2])
{
	free(clean);
	close(pfd[0]);
	close(pfd[1]);
	return (-1);
}

int	handle_heredoc(t_minishell *shell, char *file, int *sigint_status)
{
	int		pfd[2];
	char	*clean;
	int		expand;

	if (pipe(pfd) < 0)
	{
		ft_putstr_fd("minishell: pipe failed\n", STDERR_FILENO);
		return (-1);
	}
	expand = !((ft_strchr(file, '"') || ft_strchr(file, '\'')));
	clean = clean_quotes(file);
	if (!clean)
		return (clean_error(pfd));
	if (heredoc_read_loop(shell, pfd, clean, expand))
		return (heredoc_loop_error(clean, pfd));
	free(clean);
	close(pfd[1]);
	if (g_signal_status == 130)
	{
		close(pfd[0]);
		*sigint_status = 1;
		return (-1);
	}
	g_signal_status = 0;
	return (pfd[0]);
}

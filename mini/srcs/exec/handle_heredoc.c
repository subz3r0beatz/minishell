/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_heredoc.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:45:59 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 05:02:24 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*get_next(char *limiter)
{
	char	*line;

	if (isatty(STDIN_FILENO))
		line = readline("> ");
	else
		line = read_line_non_interactive(STDIN_FILENO);
	if (!line && g_signal_status != 130)
	{
		ft_putstr_fd("minishell: warning: here-document delimited by "
			"end-of-file (wanted `", STDERR_FILENO);
		ft_putstr_fd(limiter, STDERR_FILENO);
		ft_putstr_fd("')\n", STDERR_FILENO);
	}
	return (line);
}

static void	heredoc_read_loop(t_minishell *shell, int write_fd,
		char *limiter, int expand)
{
	char	*line;

	while (1)
	{
		line = get_next(limiter);
		if (!line)
			break ;
		if (!ft_strcmp(line, limiter))
		{
			free(line);
			break ;
		}
		if (expand)
			line = expand_word(shell, line);
		if (!line)
		{
			ft_putstr_fd("minishell: exec: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			break ;
		}
		ft_putendl_fd(line, write_fd);
		free(line);
	}
}

static int	malloc_error(int pfd[2], int std)
{
	close(pfd[0]);
	close(pfd[1]);
	close(std);
	ft_putstr_fd("minishell: exec: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (1);
}

int	handle_heredoc(t_minishell *shell, char *file)
{
	int		pfd[2];
	int		std;
	char	*clean;
	int		expand;

	std = dup(STDIN_FILENO);
	init_heredoc_signals();
	if (pipe(pfd) < 0)
	{
		ft_putstr_fd("minishell: exec: pipe failed\n", STDERR_FILENO);
		return (1);
	}
	expand = 0;
	if (ft_strchr(file, '"') || ft_strchr(file, '\''))
		expand = 1;
	clean = clean_quotes(file);
	if (!clean)
		return (malloc_error(pfd, std));
	heredoc_read_loop(shell, pfd[1], clean, expand);
	free(clean);
	close(pfd[1]);
	dup2(std, STDIN_FILENO);
	close(std);
	init_interactive_signals();
	if (dup2(pfd[0], STDIN_FILENO) < 0)
	{
		close(pfd[0]);
		return (1);
	}
	close(pfd[0]);
	return (0);
}

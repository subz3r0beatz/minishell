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

static char	*get_next(t_minishell *shell, char *limiter)
{
	char	*line;

	if (isatty(STDIN_FILENO))
		line = readline("> ");
	else
		line = read_line_non_interactive(STDIN_FILENO, &shell->stash);
	if (!line && g_signal_status != 130)
	{
		ft_putstr_fd("minishell: warning: here-document delimited by "
			"end-of-file (wanted `", STDERR_FILENO);
		ft_putstr_fd(limiter, STDERR_FILENO);
		ft_putstr_fd("')\n", STDERR_FILENO);
	}
	return (line);
}

static void	heredoc_read_loop(t_minishell *shell, int pfd[2],
		char *limiter, int expand)
{
	char	*line;

	while (1)
	{
		line = get_next(shell, limiter);
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
		ft_putendl_fd(line, pfd[1]);
		free(line);
	}
}

static int	malloc_error(int pfd[2])
{
	close(pfd[0]);
	close(pfd[1]);
	ft_putstr_fd("minishell: exec: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (-1);
}

int	handle_heredoc(t_minishell *shell, char *file)
{
	int		pfd[2];
	char	*clean;
	int		expand;

	init_heredoc_signals();
	if (pipe(pfd) < 0)
	{
		ft_putstr_fd("minishell: exec: pipe failed\n", STDERR_FILENO);
		return (-1);
	}
	expand = (ft_strchr(file, '"') || ft_strchr(file, '\''));
	clean = clean_quotes(file);
	if (!clean)
		return (malloc_error(pfd));
	heredoc_read_loop(shell, pfd, clean, expand);
	free(clean);
	close(pfd[1]);
	init_interactive_signals();
	if (g_signal_status == 130)
	{
		close(pfd[0]);
		shell->exit_status = 130;
		return (-1);
	}
	return (pfd[0]);
}

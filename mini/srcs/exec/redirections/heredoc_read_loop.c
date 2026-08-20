/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_read_loop.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 18:43:44 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/19 04:25:23 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	handle_history(t_minishell *shell, char *line, int *malloc_error)
{
	char	*tmp;

	*malloc_error = 0;
	if (isatty(STDIN_FILENO))
	{
		tmp = ft_strjoin_3(shell->history, "\n", line);
		if (!tmp)
		{
			*malloc_error = 1;
			ft_putstr_fd("minishell: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			free(line);
			return (1);
		}
		free(shell->history);
		shell->history = tmp;
	}
	return (0);
}

static char	*sigint_sigquit(t_minishell *shell,
	char *limiter, char *line, int *malloc_error)
{
	if (!line && *malloc_error)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	if (g_signal_status == 130)
	{
		*malloc_error = 0;
		free(line);
		return (NULL);
	}
	if (!line)
	{
		ft_putstr_fd("minishell: warning: here-document delimited by "
			"end-of-file (wanted `", STDERR_FILENO);
		ft_putstr_fd(limiter, STDERR_FILENO);
		ft_putstr_fd("')\n", STDERR_FILENO);
	}
	else if (handle_history(shell, line, malloc_error))
		return (NULL);
	return (line);
}

static char	*get_next(t_minishell *shell, char *limiter, char *buffer,
	int *malloc_error)
{
	char	*line;

	rl_event_hook = rl_signal_check;
	if (shell->input_line && *shell->input_line)
		line = extract_line(&shell->input_line, 0, malloc_error);
	else if (isatty(STDIN_FILENO))
		line = readline("> ");
	else
		line = ft_gnl(STDIN_FILENO, buffer, 128, malloc_error);
	rl_event_hook = NULL;
	return (sigint_sigquit(shell, limiter, line, malloc_error));
}

int	heredoc_read_loop(t_minishell *shell, int pfd[2],
	char *limiter, int expand)
{
	int		malloc_error;
	char	*line;
	char	buffer[128];

	buffer[0] = 0;
	while (1)
	{
		malloc_error = 0;
		line = get_next(shell, limiter, buffer, &malloc_error);
		if (!line)
			return (malloc_error);
		if (!ft_strcmp(line, limiter))
		{
			free(line);
			return (0);
		}
		if (expand)
			line = expand_word(shell, line);
		if (!line)
		{
			ft_putstr_fd("minishell: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			return (1);
		}
		ft_putendl_fd(line, pfd[1]);
		free(line);
	}
}

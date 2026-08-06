/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_loop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 23:05:28 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 03:25:47 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <stddef.h>
#include <stdio.h>

static void	process_input(t_minishell *shell, char *input)
{
	add_history(input);
	shell->tokens = lexer(input, shell->token_type_table);
	if (!shell->tokens)
		return ;
	shell->ast = parser(shell, shell->tokens);
	if (!shell->ast)
		return ;
	shell->exit_status = exec(shell, shell->ast);
	free_ast(shell->ast);
	shell->ast = NULL;
}

static char	*extract_line(char **stash)
{
	char	*line;
	char	*rest;
	size_t	i;

	if (!stash || !*stash || !**stash)
		return (NULL);
	i = 0;
	while ((*stash)[i] && (*stash)[i] != '\n')
		i++;
	line = ft_substr(*stash, 0, i);
	if (!line)
		return (NULL);
	if ((*stash)[i] == '\n')
		i++;
	rest = ft_substr(*stash, i, ft_strlen(*stash) - i);
	free(*stash);
	*stash = rest;
	if (*stash && !**stash)
	{
		free(*stash);
		*stash = NULL;
	}
	return (line);
}

static ssize_t	read_to_stash(int fd, char **stash)
{
	char	buf[4096];
	char	*tmp;
	ssize_t	bytes;

	bytes = 1;
	while (bytes > 0 && (!*stash || !ft_strchr(*stash, '\n')))
	{
		bytes = read(fd, buf, 4095);
		if (bytes < 0)
		{
			free(*stash);
			*stash = NULL;
			return (-1);
		}
		if (bytes == 0)
			break ;
		buf[bytes] = 0;
		if (!*stash)
			*stash = ft_strdup("");
		tmp = ft_strjoin(*stash, buf);
		free(*stash);
		*stash = tmp;
	}
	return (bytes);
}

char	*read_line_non_interactive(int fd, char **stash)
{
	if (fd < 0 || !stash)
		return (NULL);
	if (read_to_stash(fd, stash) < 0)
		return (NULL);
	return (extract_line(stash));
}

void	loop(t_minishell *shell)
{
	int		status;
	char	*prompt;

	while (1)
	{
		init_interactive_signals();
		if (isatty(STDIN_FILENO))
		{
			status = build_prompt(shell->env, &prompt);
			shell->input = readline(prompt);
			if (status != 2)
				free(prompt);
		}
		else
			shell->input = read_line_non_interactive(STDIN_FILENO, &shell->stash);
		if (!shell->input && isatty(STDIN_FILENO))
			ft_putstr_fd("exit\n", STDERR_FILENO);
		if (!shell->input)
			break ;
		shell->input = read_complete_input(shell, shell->input);
		if (shell->input && *shell->input)
			process_input(shell, shell->input);
		free(shell->input);
		shell->input = NULL;
	}
}

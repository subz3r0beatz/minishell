/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resolve_cmd_path.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 12:05:00 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/13 21:02:29 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*check_access(char *targets[4], char *path, int *no_malloc_error)
{
	char	*tmp;
	int		i;

	i = -1;
	while (++i < 4)
	{
		if (access(targets[i], F_OK) == 0)
		{
			if (!path)
				path = ft_strdup(targets[i]);
			else
			{
				tmp = ft_strjoin(path, ":");
				free(path);
				if (!tmp)
					return (NULL);
				path = ft_strjoin(tmp, targets[i]);
				free(tmp);
			}
			if (!path)
				return (NULL);
		}
	}
	*no_malloc_error = 1;
	return (path);
}

static char	*get_path(char **exported, int *no_malloc_error, int *has_file)
{
	char	*targets[4];
	char	*path;
	size_t	i;

	*no_malloc_error = 0;
	*has_file = 0;
	i = 0;
	while (exported[i])
	{
		if (!ft_strncmp(exported[i], "PATH=", 5))
			return (ft_strdup(&exported[i][5]));
		i++;
	}
	targets[0] = "/bin";
	targets[1] = "/usr/bin";
	targets[2] = "/usr/local/bin";
	targets[3] = "/opt/homebrew/bin";
	path = NULL;
	path = check_access(targets, path, no_malloc_error);
	return (path);
}

static char	*check_path(char **split, char *cmd,
	int *has_file, int *no_malloc_error)
{
	char	*tmp;
	char	*path;
	size_t	i;

	i = 0;
	while (split[i])
	{
		tmp = ft_strjoin(split[i], "/");
		if (!tmp)
			return (NULL);
		path = ft_strjoin(tmp, cmd);
		free(tmp);
		if (!path)
			return (NULL);
		if (access(path, F_OK) == 0)
		{
			if (access(path, X_OK) == 0)
				return (path);
			*has_file = 1;
		}
		free(path);
		i++;
	}
	*no_malloc_error = 1;
	return (NULL);
}

static void	command_error(char *cmd, int has_file)
{
	ft_putstr_fd("minishell: env: '", STDERR_FILENO);
	ft_putstr_fd(cmd, STDERR_FILENO);
	if (has_file)
		ft_putstr_fd("': Permission denied\n", STDERR_FILENO);
	else
		ft_putstr_fd("': No such file or directory\n", STDERR_FILENO);
	if (has_file)
		exit(126);
	exit(127);
}

char	*resolve_cmd_path(char **args, char **exported)
{
	char	*path;
	char	**split;
	int		has_file;
	int		no_malloc_error;

	if (ft_strchr(args[0], '/'))
		return (ft_strdup(args[0]));
	path = get_path(exported, &no_malloc_error, &has_file);
	if (path)
	{
		if (!path[0])
		{
			free(path);
			return (ft_strdup(args[0]));
		}
		split = ft_split(path, ':');
		free(path);
		if (!split)
			return (NULL);
		path = check_path(split, args[0], &has_file, &no_malloc_error);
		ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
	}
	if (!path && no_malloc_error)
		command_error(args[0], has_file);
	return (path);
}

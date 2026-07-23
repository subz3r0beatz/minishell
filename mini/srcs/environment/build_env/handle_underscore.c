/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_underscore.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 11:43:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/22 01:30:04 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	insert_argv0(t_minishell *shell, char **argv, char *value, int inherited)
{
	char	*underscore;

	if (value && value[0] == '/')
		return (0);
	if (inherited)
	{
		underscore = ft_strdup(argv[0]);
		if (!underscore)
			return (1);
		update_var_value(shell, "_", underscore);
	}
	else if (insert_new_node(shell, '_', argv[0], 1))
		return (1);
	return (0);
}

int	insert_underscore(t_minishell *shell, char *underscore, int inherited)
{
	if (inherited)
	{
		update_var_value(shell, "_", underscore);
		return (0);
	}
	else if (insert_new_node(shell, "_", underscore, 1))
	{
		free(underscore);
		return (1);
	}
	free(underscore);
	return (0);
}

int	handle_relative(t_minishell *shell, char *argv0)
{
	char	buf[PATH_MAX];
	char	*underscore;
	char	*tmp;

	if (!getcwd(buf, PATH_MAX))
		return (insert_new_node(shell, "_", argv0, 1));
	underscore = ft_strjoin(buf, "/");
	if (!underscore)
		return (1);
	tmp = ft_strjoin(underscore, argv0);
	free(underscore);
	if (!tmp)
		return (1);
	underscore = canonalize_path(buf, tmp);
	free(tmp);
	if (!underscore)
		return (1);
	if (insert_new_node(shell, "_", underscore, 1))
	{
		free(underscore);
		return (1);
	}
	free(underscore)
	return (0);
}

int	handle_path(t_minishell *shell, char **argv, char *path, int inherited)
{
	size_t	i;
	char		*underscore;
	char		*tmp;
	char		**split;

	split = ft_split(path, ':');
	if (!split)
		return (1);
	i = -1;
	while (split[++i])
	{
		free(underscore);
		tmp = ft_strjoin(split[i], "/");
		if (!tmp)
			return (1);
		underscore = ft_strjoin(tmp, argv[0]);
		free(tmp);
		if (!underscore)
			return (1);
		if (access(underscore, F_OK))
			continue ;
		ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
		return (insert_underscore(shell, underscore, inherited));
	}
	ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
	return (2);
}

int	handle_underscore(t_minishell *shell, char *argv0)
{
	char	*value;
	char	*path;

	get_var_value(shell, "_", &value);
	if (!argv0 || !argv0[0])
		argv0 = "minishell";
	if (argv[0][0] == '/')
		return (insert_new_node(shell, "_", argv0, 1));
	if (ft_strchr(argv0, '/') || get_var_value(shell, "PATH", &path))
		return (handle_relative(shell, argv0, value));
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_underscore.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 11:43:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/23 18:26:03 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
	free(underscore);
	return (0);
}

int	handle_path(t_minishell *shell, char **split, char *argv0)
{
	char	*underscore;
	char	*tmp;
	size_t	i;
	int		status;

	i = 0;
	while (split && split[i])
	{
		tmp = ft_strjoin(split[i], "/");
		if (!tmp)
			return (1);
		underscore = ft_strjoin(tmp, argv0);
		free(tmp);
		if (!underscore)
			return (1);
		if (!access(underscore, X_OK))
		{
			status = insert_new_node(shell, "_", underscore, 1);
			free(underscore);
			return (status);
		}
		free(underscore);
		i++;
	}
	return (2);
}

int	handle_underscore(t_minishell *shell, char *argv0)
{
	char	*value;
	char	*path;
	char	**split;
	int		status;

	get_var_value(shell, "_", &value);
	if (!argv0 || !argv0[0])
	{
		if (value && value[0] == '/')
			argv0 = value;
		else
			argv0 = "minishell";
	}
	if (argv0[0] == '/')
		return (insert_new_node(shell, "_", argv0, 1));
	if (ft_strchr(argv0, '/') || get_var_value(shell, "PATH", &path))
		return (handle_relative(shell, argv0));
	split = ft_split(path, ':');
	if (!split)
		return (1);
	status = handle_path(shell, split, argv0);
	ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
	if (status == 2)
		return (handle_relative(shell, argv0));
	return (status);
}

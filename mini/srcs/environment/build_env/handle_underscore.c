/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_underscore.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 11:43:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/01 04:49:14 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_relative(t_minishell *shell, char *argv0)
{
	char	buf[PATH_MAX];
	char	*underscore;
	char	*tmp;
	int		status;

	if (!getcwd(buf, PATH_MAX))
		return (insert_new_node(shell, "_", argv0, 1));
	tmp = ft_strjoin_3(buf, "/", argv0);
	if (!tmp)
		return (1);
	underscore = canonalize_path(shell, buf, tmp);
	free(tmp);
	if (!underscore)
		return (1);
	status = insert_new_node(shell, "_", underscore, 1);
	free(underscore);
	return (status);
}

static int	handle_path(t_minishell *shell, char **split, char *argv0)
{
	char	*underscore;
	size_t	i;
	int		status;

	i = 0;
	while (split && split[i])
	{
		underscore = ft_strjoin_3(split[i], "/", argv0);
		if (!underscore)
		{
			ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
			return (1);
		}
		if (!access(underscore, X_OK))
		{
			ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
			status = insert_new_node(shell, "_", underscore, 1);
			free(underscore);
			return (status);
		}
		free(underscore);
		i++;
	}
	ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
	return (handle_relative(shell, argv0));
}

int	handle_underscore(t_minishell *shell)
{
	char	*argv0;
	char	*value;
	char	*path;
	char	**split;

	argv0 = shell->argv0;
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
	return (handle_path(shell, split, argv0));
}

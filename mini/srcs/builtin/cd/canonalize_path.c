/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   canonalize_path.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 19:31:13 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/18 00:07:49 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	clean_split(char **split)
{
	size_t	i;

	i = 0;
	while (split[i])
	{
		if (split[i][0] == '.' && !split[i][1])
		{
			free(split[i]);
			ft_mem_shift(&split[i], 1, sizeof(char *), -1);
		}
		else if (split[i][0] == '.' && split[i][1] == '.' && !split[i][2])
		{
			free(split[i]);
			if (i > 0)
			{
				i--;
				free(split[i]);
				ft_mem_shift(&split[i], 2, sizeof(char *), -1);
			}
			else
				ft_mem_shift(&split[i], 1, sizeof(char *), -1);
		}
		else
			i++;
	}
}

static char	*get_raw_path(char *pwd, char *path)
{
	if (path[0] == '/')
		return (ft_strdup(path));
	if (!ft_strcmp(pwd, "/"))
		return (ft_strjoin("/", path));
	if (!ft_strcmp(pwd, "//"))
		return (ft_strjoin("//", path));
	return (ft_strjoin_3(pwd, "/", path));
}

static char	*add_slashes(t_minishell *shell, char **split, int is_double)
{
	char	*tmp;
	char	*ret;

	if (!split[0])
	{
		ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
		if (is_double)
			return (ft_strdup("//"));
		return (ft_strdup("/"));
	}
	tmp = ft_join_split_prefix((const char **)split, "/");
	ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
	if (!tmp)
		return (NULL);
	shell->double_root = (tmp[0] == '/' && tmp[1] == '/' && tmp[2] != '/');
	if (!is_double && !shell->double_root)
		return (tmp);
	ret = ft_strjoin("/", tmp);
	free(tmp);
	return (ret);
}

char	*canonalize_path(t_minishell *shell, char *pwd, char *path)
{
	char	*ret;
	char	**split;
	int		is_double;

	ret = get_raw_path(pwd, path);
	if (!ret)
		return (NULL);
	is_double = 0;
	if (ret[0] == '/' && ret[1] == '/' && ret[2] != '/')
		is_double = 1;
	split = ft_split(ret, '/');
	free(ret);
	if (!split)
		return (NULL);
	clean_split(split);
	return (add_slashes(shell, split, is_double));
}

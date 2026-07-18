/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   canonalize_path.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 19:31:13 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 04:03:50 by fldumas-         ###   ########.fr       */
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

char	*canonalize_path(char *pwd, char *path)
{
	char	*ret;
	char	*tmp;
	char	**split;
	size_t	len;

	if (path[0] == '/')
		ret = ft_strdup(path);
	else
	{
		tmp = ft_strjoin(pwd, "/");
		if (!tmp)
			return (NULL);
		ret = ft_strjoin(tmp, path);
		free(tmp);
	}
	if (!ret)
		return (NULL);
	split = ft_split(ret, '/');
	free(ret);
	if (!split)
		return (NULL);
	len = ft_memlen(split, sizeof(char *));
	clean_split(split);
	ret = ft_join_split_prefix((const char **)split, "/");
	ft_free_matrix(split, len);
	return (ret);
}

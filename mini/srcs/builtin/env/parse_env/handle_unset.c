/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_unset.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 17:12:20 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 15:51:14 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	unset_env_var(char ***exported, char *unset, size_t *exported_len)
{
	size_t	i;
	size_t	len;

	i = 0;
	len = ft_strlen(unset);
	while ((*exported)[i])
	{
		if (!ft_strncmp((*exported)[i], unset, len)
			&& (*exported)[i][len] == '=')
		{
			free((*exported)[i]);
			ft_mem_shift(&(*exported)[i], 1, sizeof(char *), -1);
			(*exported_len)--;
			return ;
		}
		i++;
	}
}

static char	*parse_key(char **args, size_t *i, size_t *j)
{
	char	*unset;

	(*j)++;
	if (!args[*i][*j] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'u'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (NULL);
	}
	if (args[*i][*j])
		unset = &args[*i][*j];
	else
		unset = args[++*i];
	if (!*unset || ft_strchr(unset, '='))
	{
		ft_putstr_fd("minishell: env: cannot unset '", STDERR_FILENO);
		ft_putstr_fd(unset, STDERR_FILENO);
		ft_putstr_fd("': Invalid argument\n", STDERR_FILENO);
		return (NULL);
	}
	return (unset);
}

int	handle_unset(char **matrices[2], t_max_uints *max_uints)
{
	size_t	*i;
	size_t	*j;
	char	*unset;

	i = &max_uints->i;
	j = &max_uints->j;
	unset = parse_key(matrices[1], i, j);
	if (!unset)
		return (1);
	unset_env_var(&matrices[0], unset, &max_uints->exported_len);
	*j = ft_strlen(matrices[1][*i]) - 1;
	return (0);
}

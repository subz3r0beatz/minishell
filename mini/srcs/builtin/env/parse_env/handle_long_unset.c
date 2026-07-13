/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_long_unset.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 19:19:06 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/13 20:14:16 by fldumas-         ###   ########.fr       */
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
		ft_putstr_fd("minishell: env: option '--unset' requires an argument\n"
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

int	handle__long_unset(char **args, char ***exported, size_t max_uints[4])
{
	size_t	*i;
	size_t	*j;
	size_t	*exported_len;
	char	*unset;

	i = max_uints[0];
	j = max_uints[1];
	exported_len = max_uints[2];
	unset = parse_key(args, i, j);
	if (!unset)
		return (1);
	unset_env_var(exported, unset, exported_len);
	*j = ft_strlen(args[*i]) - 1;
	return (0);
}

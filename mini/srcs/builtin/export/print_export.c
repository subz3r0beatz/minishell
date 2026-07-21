/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_export.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 15:17:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/21 16:10:02 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	bubble_sort_env(t_env **sorted_env)
{
	size_t	i;
	size_t	j;

	i = 0;
	while (sorted_env[i])
	{
		j = i + 1;
		while (sorted_env[j])
		{
			if (ft_strcmp(sorted_env[i]->key, sorted_env[j]->key) > 0)
				ft_swap((void **)&sorted_env[i], (void **)&sorted_env[j]);
			j++;
		}
		i++;
	}
}

static t_env	**sort_env(t_robin *env)
{
	size_t	i;
	size_t	j;
	t_env	**sorted_env;

	sorted_env = malloc(sizeof(t_env *) * (env->count + 1));
	if (!sorted_env)
	{
		ft_putstr_fd("minishell: export: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	i = 0;
	j = 0;
	while (i < env->capacity)
	{
		if (env->ctrl[i] > 0)
		{
			sorted_env[j] = (t_env *)env->data[i].value;
			j++;
		}
		i++;
	}
	sorted_env[j] = NULL;
	bubble_sort_env(sorted_env);
	return (sorted_env);
}

static void	print_value(char *value, int fd_out)
{
	size_t	i;
	size_t	j;

	i = 0;
	j = 0;
	while (value[i])
	{
		if (value[i] == '"' || value[i] == '\\'
			|| value[i] == '$' || value[i] == '`')
		{
			write(fd_out, &value[j], i - j);
			ft_putchar_fd('\\', fd_out);
			j = i;
		}
		i++;
	}
	write(fd_out, &value[j], i - j);
}

int	print_export(t_robin *env, int fd_out)
{
	size_t	i;
	t_env	**sorted_env;

	sorted_env = sort_env(env);
	if (!sorted_env)
		return (2);
	i = 0;
	while (sorted_env[i])
	{
		if (sorted_env[i]->is_exported)
		{
			ft_putstr_fd("declare -x ", fd_out);
			ft_putstr_fd(sorted_env[i]->key, fd_out);
			if (sorted_env[i]->value)
			{
				ft_putstr_fd("=\"", fd_out);
				print_value(sorted_env[i]->value, fd_out);
				ft_putchar_fd('"', fd_out);
			}
			ft_putchar_fd('\n', fd_out);
		}
		i++;
	}
	free(sorted_env);
	return (0);
}

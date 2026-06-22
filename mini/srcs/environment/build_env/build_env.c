/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 18:17:23 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/20 21:23:10 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_robin_node	parse_var(t_robin *env, char *str)
{
	size_t			i;
	char			*key;
	char			*value;
	t_robin_node	robin_node;

	i = 0;
	while (str[i] && str[i] != '=')
		i++;
	key = ft_substr(str, 0, i);
	value = NULL;
	if (str[i])
		value = ft_strdup(str + i + 1);
	if (!key || (str[i] && !value))
	{
		free(key);
		free(value);
		return (NULL);
	}
	robin_node = create_node(env, key, value, 1);
	if (!robin_node.value)
		return (NULL);
	return (robin_node);
}

static t_robin	*cpy_env(char **envp)
{
	size_t			i;
	size_t			cap;
	t_robin			*env;
	t_robin_node	robin_node;

	i = 0;
	cap = 4096;
	while (envp && envp[i++] && cap < SIZE_MAX / 2)
		if (cap < i)
			cap *= 2;
	if (cap < i)
		return (NULL);
	env = robin_init(cap, fnv1a, ft_strcmp, delete_node);
	if (!env)
		return (NULL);
	i = 0;
	while (envp && envp[i])
	{
		robin_node = parse_var(env, envp[i]);
		if (!robin_node)
			return (NULL);
		robin_insert(env, robin_node);
		i++;
	}
	return (env);
}

t_robin	*build_env(char **envp)
{
	t_robin			*env;

	env = cpy_env(envp, cap);
	if (!env)
		return (NULL);
	add_basics(env);
	return (env);
}

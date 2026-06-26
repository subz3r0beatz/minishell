/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 18:17:23 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 18:57:10 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	get_capacity(char **envp)
{
	size_t	i;
	size_t	cap;

	i = 0;
	cap = 4096;
	while (envp && envp[i] && cap < SIZE_MAX / 2)
	{
		if (cap < i)
			cap *= 2;
		i++;
	}
	if (cap < i)
		return (0);
	return (cap);
}

static int	insert_new_var(t_robin *env, char *key, char *value)
{
	t_robin_node	robin_node;

	robin_node = create_node(env, key, value, 1);
	if (!robin_node.key || !robin_node.value)
	{
		robin_free(env);
		return (1);
	}
	if (robin_insert(env, robin_node))
	{
		env->del_function(robin_node.key, robin_node.value);
		robin_free(env);
		return (1);
	}
	return (0);
}

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
		robin_node.key = NULL;
		return (1);
	}
	return (insert_new_var(env, key, value));
}

static t_robin	*cpy_env(char **envp)
{
	size_t			i;
	size_t			cap;
	t_robin			*env;
	t_robin_node	robin_node;

	cap = get_capacity(envp);
	if (!cap)
		return (NULL);
	env = robin_init(cap, fnv1a, ft_strcmp, delete_node);
	if (!env)
		return (NULL);
	i = 0;
	while (envp && envp[i])
	{
		if (parse_var(env, envp[i]))
			return (NULL);
		i++;
	}
	return (env);
}

t_robin	*build_env(char **envp)
{
	t_robin			*env;

	env = cpy_env(envp);
	if (!env)
		return (NULL);
	if (handle_pwd(env) || handle_oldpwd(env)
		|| handle_shlvl(env) || handle_underscore(env))
	{
		robin_free(env);
		return (NULL);
	}
	return (env);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 18:17:23 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 20:39:06 by fldumas-         ###   ########.fr       */
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

static int	insert_new_var(t_minishell *shell, char *key, char *value)
{
	t_robin_node	robin_node;

	robin_node = create_node(shell->env, key, value, 1);
	if (!robin_node.key || !robin_node.value)
	{
		robin_free(minishell->env);
		return (1);
	}
	if (robin_insert(shell->env, robin_node))
	{
		shell->env->del_function(robin_node.key, robin_node.value);
		robin_free(shell->env);
		return (1);
	}
	shell->exported_count++;
	return (0);
}

static int	parse_var(t_minishell *shell, char *str)
{
	size_t	i;
	char	*key;
	char	*value;

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
		return (1);
	}
	if (insert_new_var(shell, key, value))
		return (1);
	return (0);
}

static int	cpy_env(t_minishell *shell, char **envp)
{
	size_t	i;
	size_t	cap;

	cap = get_capacity(envp);
	if (!cap)
		return (1);
	shell->env = robin_init(cap, fnv1a, ft_strcmp, delete_node);
	if (!shell->env)
		return (1);
	i = 0;
	while (envp && envp[i])
	{
		if (parse_var(shell, envp[i]))
			return (1);
		i++;
	}
	return (0);
}

int	build_env(t_minishell *shell, char **envp)
{
	if (cpy_env(shell, envp))
		return (1);
	if (handle_pwd(shell) || handle_oldpwd(shell)
		|| handle_shlvl(shell) || handle_underscore(shell))
	{
		robin_free(shell->env);
		return (1);
	}
	return (0);
}

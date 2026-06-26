/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unexport_vars.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 15:54:38 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 18:40:21 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	is_valid_key(char *str)
{
	size_t	i;

	if (ft_isalpha(str[0]) || str[0] == '_')
	{
		i = 1;
		while (str[i] && str[i] != '=')
		{
			if (!ft_isalnum(str[i]) && str[i] != '_')
				break ;
			i++;
		}
		if (str[i] == '=' || !str[i])
			return (i);
	}
	ft_putstr_fd("minishell: export: ", STDERR_FILENO);
	ft_putstr_fd(str, STDERR_FILENO);
	ft_putstr_fd(": not a valid identifier\n", STDERR_FILENO);
	return (0);
}

static int	lookup_var(t_robin *env, char *key, char *value)
{
	t_robin_node	*robin_node;

	robin_node = robin_search(env, key);
	if (!robin_node)
		return (1);
	free(key);
	if (value)
	{
		free(((t_env *)robin_node->value)->value);
		((t_env *)robin_node->value)->value = value;
	}
	((t_env *)robin_node->value)->is_exported = 0;
	return (0);
}

static int	insert_new_var(t_robin *env, char *key, char *value)
{
	t_robin_node	robin_node;

	robin_node = create_node(env, key, value, 0);
	if (!robin_node.key || !robin_node.value)
		return (2);
	if (robin_insert(env, robin_node))
	{
		env->del_function(robin_node.key, robin_node.value);
		return (2);
	}
	return (0);
}

static int	insert_var(t_robin *env, char *str)
{
	size_t			i;
	char			*key;
	char			*value;

	i = is_valid_key(str);
	if (!i)
		return (1);
	key = ft_substr(str, 0, i);
	value = NULL;
	if (str[i])
		value = ft_strdup(str + i + 1);
	if (!key || (str[i] && !value))
	{
		free(key);
		free(value);
		return (2);
	}
	if (!lookup_var(env, key, value))
		return (0);
	if (!str[i])
	{
		free(key);
		return (0);
	}
	return (insert_new_var(env, key, value));
}

int	unexport_vars(t_robin *env, char **args)
{
	int		status;
	int		ret;
	size_t	i;

	ret = 0;
	i = 0;
	while (args[i])
	{
		status = insert_var(env, args[i]);
		if (status == 1)
			ret = 1;
		if (status == 2)
			return (status);
		i++;
	}
	return (ret);
}

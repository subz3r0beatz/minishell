/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_shlvl.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 09:44:46 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 19:00:18 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	check_valid_shlvl(char *str)
{
	size_t	i;

	i = 0;
	if (str[i] == '+' || str[i] == '-')
		i++;
	if (!str[i])
		return (1);
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (1);
		i++;
	}
	return (0);
}

static int	increment_shlvl(t_robin_node *robin_node)
{
	int		lvl;
	char	*new_val;
	t_env	*env;

	env = (t_env *)robin_node->value;
	if (!env->value || check_valid_shlvl(env->value))
		new_val = ft_strdup("1");
	else
	{
		lvl = ft_atoi(env->value) + 1;
		if (lvl < 0)
			lvl = 0;
		new_val = ft_itoa(lvl);
	}
	if (!new_val)
		return (1);
	free(env->value);
	env->value = new_val;
	return (0);
}

static int	insert_shlvl(t_robin *env, char *shlvl, char *value)
{
	t_robin_node	robin_node;

	robin_node = create_node(env, shlvl, value, 1);
	if (!robin_node.key || !robin_node.value)
		return (1);
	if (robin_insert(env, robin_node))
	{
		env->del_function(robin_node.key, robin_node.value);
		return (1);
	}
	return (0);
}

int	handle_shlvl(t_robin *env)
{
	char			*shlvl;
	char			*value;
	t_robin_node	*robin_node;
	t_robin_node	new_node;

	robin_node = robin_search(env, "SHLVL");
	if (!robin_node)
	{
		shlvl = ft_strdup("SHLVL");
		value = ft_strdup("1");
		if (!shlvl || !value)
		{
			free(shlvl);
			free(value);
			return (1);
		}
		return (insert_shlvl(env, shlvl, value));
	}
	return (increment_shlvl(robin_node));
}

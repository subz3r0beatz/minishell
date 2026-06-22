/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_node.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:45:06 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/18 18:00:12 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_robin_node	create_node(t_robin *env,
		char *key, char *value, int is_exported)
{
	t_env			*env_node;
	t_robin_node	robin_node;

	env_node = create_env_node(key, value, is_exported);
	if (!env_node)
	{
		free(key);
		if (value)
			free(value);
		robin_node.key = NULL;
		robin_node.value = env_node;
		robin_node.hash = 0;
		return (robin_node);
	}
	robin_node.key = key;
	robin_node.value = env_node;
	robin_node.hash = env->hash_function(key);
	return (robin_node);
}

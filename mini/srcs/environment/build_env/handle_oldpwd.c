/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_oldpwd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 11:38:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 18:58:08 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_oldpwd(t_robin *env)
{
	char			*oldpwd;
	t_robin_node	*robin_node;
	t_robin_node	new_node;

	robin_node = robin_search(env, "OLDPWD");
	if (!robin_node)
	{
		oldpwd = ft_strdup("OLDPWD");
		if (!oldpwd)
			return (1);
		new_node = create_node(env, oldpwd, NULL, 1);
		if (!new_node.key || !new_node.value)
			return (1);
		if (robin_insert(env, new_node))
		{
			env->del_function(new_node.key, new_node.value);
			return (1);
		}
		return (0);
	}
	free(((t_env *)robin_node->value)->value);
	((t_env *)robin_node->value)->value = NULL;
	return (0);
}

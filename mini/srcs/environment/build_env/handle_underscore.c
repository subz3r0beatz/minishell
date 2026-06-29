/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_underscore.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 11:43:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 19:06:43 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_underscore(t_minishell *shell)
{
	char			*underscore;
	char			*value;
	t_robin_node	new_node;

	if (robin_search(env, "_"))
		return (0);
	underscore = ft_strdup("_");
	value = ft_strdup("./minishell");
	if (!underscore || !value)
	{
		free(underscore);
		free(value);
		return (1);
	}
	new_node = create_node(env, underscore, value, 1);
	if (!new_node.key || !new_node.value)
		return (1);
	if (robin_insert(env, new_node))
	{
		env->del_function(new_node.key, new_node.value);
		return (1);
	}
	shell->exported_count++;
	return (0);
}

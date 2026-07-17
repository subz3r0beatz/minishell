/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_var_value.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 17:21:19 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 21:29:57 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	get_var_value(t_minishell *shell, char *key, char **ptr)
{
	t_robin_node	*robin_node;

	robin_node = robin_search(shell->env, key);
	if (!robin_node)
		return (1);
	*ptr = ((t_env *)robin_node->value)->value;
	return (0);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   update_var_value.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 17:26:32 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 09:07:55 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	update_var_value(t_minishell *shell, char *key, char *value)
{
	t_robin_node	*robin_node;

	robin_node = robin_search(shell->env, key);
	if (!robin_node)
		return (1);
	if (((t_env *)robin_node->value)->value && !value
		&& ((t_env *)robin_node->value)->is_exported)
		shell->exported_count--;
	else if (!((t_env *)robin_node->value)->value && value
		&& ((t_env *)robin_node->value)->is_exported)
		shell->exported_count++;
	free(((t_env *)robin_node->value)->value);
	((t_env *)robin_node->value)->value = value;
	return (0);
}

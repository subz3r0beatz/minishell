/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   insert_new_node.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:46:24 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/21 19:46:26 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	insert_new_node(t_minishell *shell, char *key,
	char *value, int is_exported)
{
	t_robin_node	node;
	char			*key_dup;
	char			*value_dup;

	key_dup = ft_strdup(key);
	if (!key_dup)
		return (1);
	value_dup = ft_strdup(value);
	if (value && !value_dup)
	{
		free(key_dup);
		return (1);
	}
	node = create_node(shell->env, key_dup, value_dup, is_exported);
	if (!node.key || !node.value)
		return (1);
	if (value && is_exported && !check_exported(shell, key))
		shell->exported_count++;
	else if ((!value || !is_exported) && check_exported(shell, key))
		shell->exported_count--;
	if (robin_insert(shell->env, node))
		return (1);
	return (0);
}

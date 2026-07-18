/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_oldpwd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 11:38:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 02:16:54 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_oldpwd(t_minishell *shell)
{
	char			*oldpwd;
	t_robin_node	*robin_node;
	t_robin_node	new_node;

	robin_node = robin_search(shell->env, "OLDPWD");
	if (robin_node)
		return (0);
	oldpwd = ft_strdup("OLDPWD");
	if (!oldpwd)
		return (1);
	new_node = create_node(shell->env, oldpwd, NULL, 1);
	if (!new_node.key || !new_node.value)
		return (1);
	if (robin_insert(shell->env, new_node))
	{
		shell->env->del_function(new_node.key, new_node.value);
		return (1);
	}
	return (0);
}

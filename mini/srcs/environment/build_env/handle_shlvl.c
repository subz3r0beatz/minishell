/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_shlvl.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 09:44:46 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 04:28:33 by fldumas-         ###   ########.fr       */
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
	while (str[i] && ft_isdigit(str[i]))
		i++;
	return (str[i] != 0);
}

int	handle_shlvl(t_minishell *shell)
{
	char	*value;
	int		lvl;
	int		status;

	if (get_var_value(shell, "SHLVL", &value) || !value
		|| check_valid_shlvl(value))
		return (insert_new_node(shell, "SHLVL", "1", 1));
	lvl = ft_atoi(value) + 1;
	lvl = lvl & ~(lvl >> 31);
	value = ft_itoa(lvl);
	if (!value)
		return (1);
	status = insert_new_node(shell, "SHLVL", value, 1);
	free(value);
	return (status);
}

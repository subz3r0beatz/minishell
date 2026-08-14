/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_builtin_func_table.c                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 17:07:16 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:31:52 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	init_builtin_func_table(int (*builtin_func_table[7])(t_minishell *shell,
		char **args))
{
	builtin_func_table[0] = ft_echo;
	builtin_func_table[1] = ft_env;
	builtin_func_table[2] = ft_exit;
	builtin_func_table[3] = ft_export;
	builtin_func_table[4] = ft_unset;
	builtin_func_table[5] = ft_cd;
	builtin_func_table[6] = ft_pwd;
}

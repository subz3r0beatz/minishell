/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 11:47:28 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 15:48:20 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	exit_env(char **matrices[2], t_flags *flags,
	t_max_uints *max_uints, int exit_code)
{
	if (!matrices)
		return (exit_code);
	if (matrices[0])
		ft_free_matrix(matrices[0], max_uints->exported_len);
	if (matrices[1])
		ft_free_matrix(matrices[1], max_uints->args_len);
	if (!flags)
		return (exit_code);
	if (flags->chdir_path)
		free(flags->chdir_path);
	if (flags->custom_argv0)
		free(flags->custom_argv0);
	return (exit_code);
}

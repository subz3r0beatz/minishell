/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_long_chdir_path.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 19:15:56 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 15:51:58 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	parse_long_chdir_path(char **args, t_flags *flags, size_t *i, size_t *j)
{
	(*j)++;
	if (!args[*i][*j] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option '--chdir' requires an argument\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if (flags->chdir_path)
		free(flags->chdir_path);
	if (args[*i][*j - 1] == '=' || args[*i][*j])
		flags->chdir_path = ft_strdup(&args[*i][*j]);
	else
		flags->chdir_path = ft_strdup(args[++*i]);
	if (!flags->chdir_path)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	*j = ft_strlen(args[*i]) - 1;
	return (0);
}

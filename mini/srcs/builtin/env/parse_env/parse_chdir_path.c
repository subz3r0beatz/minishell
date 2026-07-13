/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_chdir_path.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 17:12:56 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/09 17:13:02 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	parse_chdir_path(char **args, t_flags *flags, size_t *i, size_t *j)
{
	(*j)++;
	if (!args[*i][*j] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'C'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if (args[*i][*j])
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

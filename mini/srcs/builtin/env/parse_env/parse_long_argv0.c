/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_long_argv0.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 19:13:33 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 16:44:47 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	parse_long_argv0(char **args, t_flags *flags, size_t *i, size_t *j)
{
	(*j)++;
	if (!args[*i][*j] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option '--argv0' requires an argument\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if (flags->custom_argv0)
		free(flags->custom_argv0);
	if (args[*i][*j - 1] == '=' || args[*i][*j])
		flags->custom_argv0 = ft_strdup(&args[*i][*j]);
	else
		flags->custom_argv0 = ft_strdup(args[++*i]);
	if (!flags->custom_argv0)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	*j = ft_strlen(args[*i]) - 1;
	return (0);
}

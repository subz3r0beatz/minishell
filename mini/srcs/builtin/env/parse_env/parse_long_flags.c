/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_long_flags.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 18:15:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 17:17:39 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <stddef.h>

static int	long_flag_error(char *arg)
{
	ft_putstr_fd("minishell: env: invalid option '", STDERR_FILENO);
	ft_putstr_fd(arg, STDERR_FILENO);
	ft_putstr_fd("'\nTry 'env --help' for more information.\n",
		STDERR_FILENO);
	return (1);
}

static int	parse_flag(char *arg, char *str, size_t *j)
{
	size_t	i;

	i = 0;
	if (!arg[i] || arg[i] == '=')
		return (1);
	while (arg[i] && str[i] && arg[i] != '=')
	{
		if (arg[i] != str[i])
			return (1);
		i++;
	}
	if (arg[i] && arg[i] != '=')
		return (1);
	if (arg[i] == '=')
		*j = 2 + i;
	else
		*j = 1 + i;
	return (0);
}

static int	handle_bool_flags(char *arg, t_flags *flags, size_t *j)
{
	if (!parse_flag(&arg[2], "ignore-environment", j)
		&& arg[*j] != '=' && !arg[*j + 1])
		flags->ignore_env = 1;
	else if (!parse_flag(&arg[2], "null", j)
		&& arg[*j] != '=' && !arg[*j + 1])
		flags->null_term = 1;
	else if (!parse_flag(&arg[2], "help", j)
		&& arg[*j] != '=' && !arg[*j + 1])
		flags->print_help = 1;
	else
		return (0);
	return (1);
}

static int	handle_helpers(char **matrices[2],
	t_flags *flags, t_max_uints *max_uints)
{
	int		ret;
	size_t	*i;
	size_t	*j;

	i = &max_uints->i;
	j = &max_uints->j;
	ret = 0;
	if (!parse_flag(&matrices[1][*i][*j], "chdir", j))
		ret = parse_long_chdir_path(matrices[1], flags, i, j);
	else if (!parse_flag(&matrices[1][*i][*j], "argv0", j))
		ret = parse_long_argv0(matrices[1], flags, i, j);
	else if (!parse_flag(&matrices[1][*i][*j], "split-string", j))
		ret = handle_long_split(&matrices[1], i, j, &max_uints->args_len);
	else if (!parse_flag(&matrices[1][*i][*j], "unset", j))
		ret = handle_long_unset(matrices, max_uints);
	else
		ret = long_flag_error(matrices[1][*i]);
	return (ret);
}

int	parse_long_flags(char **matrices[2],
	t_flags *flags, t_max_uints *max_uints)
{
	int		ret;
	size_t	*i;
	size_t	*j;

	i = &max_uints->i;
	j = &max_uints->j;
	ret = 0;
	*j = 2;
	if (handle_bool_flags(matrices[1][*i], flags, j))
		return (0);
	else
		ret = handle_helpers(matrices, flags, max_uints);
	if (ret)
		return (1);
	return (0);
}

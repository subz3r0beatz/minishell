/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_long_flags.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 18:15:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 18:21:14 by fldumas-         ###   ########.fr       */
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
	if (!parse_flag(&arg[2], "ignore-environment", j))
		flags->ignore_env = 1;
	else if (!parse_flag(&arg[2], "null", j))
		flags->null_term = 1;
	else if (!parse_flag(&arg[2], "help", j))
		flags->print_help = 1;
	else
		return (0);
	return (1);

}

static int	handle_helpers(char ***matrices[2], t_flags *flags,
	size_t *max_uints[4])
{
	int		ret;
	size_t	*i;
	size_t	*j;
	char	***args;
	char	***exported;

	i = &max_uints[0];
	j = &max_uints[1];
	ret = 0;
	args = matrices[0];
	exported = matrices[1];
	if (!parse_flag(&(*args)[*i][*j], "chdir", j))
		ret = parse_chdir_path(*args, flags, i, j);
	else if (!parse_flag(&(*args)[*i][*j], "argv0", j))
		ret = parse_argv0(*args, flags, i, j);
	else if (!parse_flag(&(*args)[*i][*j], "split-string", j))
		ret = handle_split(args, i, j, max_uints[3]);
	else if (!parse_flag(&(*args)[*i][*j], "unset", j))
		ret = handle_unset(*args, exported, max_uints);
	else
	 	ret = long_flag_error((*args)[*i]);
	return (ret);
}

static int	check_long_flags(char ***args, t_flags *flags,
	char ***exported, size_t *max_uints[4])
{
	int		ret;
	size_t	j;
	char	***matrices[2];

	i = (*max_uints)[0];
	max_uints[1] = &j;
	ret = 0;
	j = 2;
	if (handle_bool_flags((*args)[*i], flags, &max_uints[1]))
		return (0);
	else
	{
		matrices[0] = args;
		matrices[1] = exported;
		ret = handle_helpers(matrices, flags, max_uints);
	}
	if (ret)
		return (1);
	return (0);
}

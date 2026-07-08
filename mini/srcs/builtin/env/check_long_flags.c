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
	return (2);
}

static int	parse_flag(char *arg, char *str, size_t *j)
{
	size_t	i;

	i = 0;
	while (arg[i] && str[i] && arg[i] != '=')
	{
		if (arg[i] != str[i])
			return (1);
		i++;
	}
	if (arg[i] == '=' || (arg[i - 1] == str[i - 1] && !arg[i]))
	{
		*j += i;
		return (0);
	}
	return (1);
}

static int	handle_bool_flags(char **args, t_flags *flags, size_t *i, size_t *j)
{
	if (!parse_flag(&args[*i][*j], "ignore-environment", j))
		flags->ignore_env = 1;
	else if (!parse_flag(&args[*i][*j], "null", j))
		flags->null_term = 1;
	else if (!parse_flag(&args[*i][*j], "help", j))
		flags->print_help = 1;
	else
		return (0);
	return (1);

}

static int	handle_helpers(char ***matrices[2], t_flags *flags, size_t *i, size_t *j)
{
	int		ret;
	char	***args;
	char	***exported;

	ret = 0;
	args = matrices[0];
	exported = matrices[1];
	if (!parse_flag(&args[*i][*j], "chdir", j))
		ret = parse_chdir_path(args, flags, i, j);
	else if (!parse_flag(&args[*i][*j], "argv0", j))
		ret = parse_argv0(args, flags, i, j);
	else if (!parse_flag(&args[*i][*j], "split-string", j))
		ret = handle_string(args, i, j);
	else if (!parse_flag(&args[*i][*j], "unset", j))
		ret = handle_unset(args, exported, i, j);
	else
	 	ret = long_flag_error(agrs[*i]);
	return (ret);
}

static int	check_long_flags(char **args, t_flags *flags,
	char ***exported, size_t *i)
{
	int		ret;
	size_t	j;
	char	***matrices[2];

	ret = 0;
	j = 2;
	if (handle_bool_flags(args, flags, i, &j))
		return (0);
	else
	{
		matrices[0] = &args;
		matrices[1] = exported;
		ret = handle_helpers(matrices, flags, i, &j);
	}
	if (ret == 2)
		ft_free_matrix(*exported, ft_memlen(*exported, sizeof(char *)));
	if (ret)
		return (1);
	}
	return (0);
}

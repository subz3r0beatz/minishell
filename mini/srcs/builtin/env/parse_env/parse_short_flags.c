/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_short_flags.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 18:21:24 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 15:52:26 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	flag_error(char c)
{
	ft_putstr_fd("minishell: env: invalid option -- '", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd("'\nTry 'env --help' for more information.\n",
		STDERR_FILENO);
	return (1);
}

static int	handle_helpers(char	**matrices[2],
	t_flags *flags, t_max_uints *max_uints)
{
	int		ret;
	size_t	*i;
	size_t	*j;

	i = &max_uints->i;
	j = &max_uints->j;
	ret = 0;
	if (matrices[1][*i][*j] == 'C')
		ret = parse_chdir_path(matrices[1], flags, i, j);
	else if (matrices[1][*i][*j] == 'a')
		ret = parse_argv0(matrices[1], flags, i, j);
	else if (matrices[1][*i][*j] == 'S')
		ret = handle_split(&matrices[1], i, j, &max_uints->args_len);
	else if (matrices[1][*i][*j] == 'u')
		ret = handle_unset(matrices, max_uints);
	else
		ret = flag_error(matrices[1][*i][*j]);
	return (ret);
}

int	parse_short_flags(char **matrices[2],
	t_flags *flags, t_max_uints *max_uints)
{
	int		ret;
	size_t	*i;
	size_t	*j;

	i = &max_uints->i;
	j = &max_uints->j;
	ret = 0;
	*j = 1;
	while (matrices[1][*i][*j])
	{
		if (matrices[1][*i][*j] == 'i')
			flags->ignore_env = 1;
		else if (matrices[1][*i][*j] == '0')
			flags->null_term = 1;
		else
			ret = handle_helpers(matrices, flags, max_uints);
		if (ret)
			return (1);
		(*j)++;
	}
	return (0);
}

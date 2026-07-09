/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_flags.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 18:21:24 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 21:50:00 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <stddef.h>

static int	flag_error(char c)
{
	ft_putstr_fd("minishell: env: invalid option -- '", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd("'\nTry 'env --help' for more information.\n",
		STDERR_FILENO);
	return (1);
}

static int	handle_helpers(char	***matrices[2], t_flags *flags,
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
	if ((*args)[*i][*j] == 'C')
		ret = parse_chdir_path(*args, flags, i, j);
	else if ((*args)[*i][*j] == 'a')
		ret = parse_argv0(*args, flags, i, j);
	else if ((*args)[*i][*j] == 'S')
		ret = handle_split(args, i, j, &max_uints[3]);
	else if ((*args)[*i][*j] == 'u')
		ret = handle_unset(*args, exported, max_uints);
	else
		ret = flag_error((*args)[*i][*j]);
	return (ret);
}

static int	check_flags(char ***args, t_flags *flags,
	char ***exported, size_t *max_uints[4])
{
	int		ret;
	size_t	j;
	char	***matrices[2];

	i = (*max_uints)[0];
	max_uints[1] = &j;
	ret = 0;
	j = 0;
	while ((*args)[*i][++j])
	{
		if ((*args)[*i][j] == 'i')
			flags->ignore_env = 1;
		else if ((*args)[*i][j] == '0')
			flags->null_term = 1;
		else
		{
			matrices[0] = args;
			matrices[1] = exported;
			ret = handle_helpers(matrices, flags, i, max_uints);
		}
		if (ret)
			return (1);
	}
	return (0);
}

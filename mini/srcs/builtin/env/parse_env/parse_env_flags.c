/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_env_flags.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 16:47:49 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 17:02:18 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

size_t	parse_env_flags(char **matrices[2],
	t_flags *flags, t_max_uints *max_uints)
{
	size_t		*i;

	i = &max_uints->i;
	*i = 1;
	while (matrices[1][*i] && matrices[1][*i][0] == '-' && matrices[1][*i][1])
	{
		if (matrices[1][*i][1] == '-')
		{
			if (!matrices[1][*i][2])
				return (*i + 1);
			else if (parse_long_flags(matrices, flags, max_uints))
				return (0);
			if (flags->print_help)
				return (*i + 1);
		}
		else if (parse_short_flags(matrices, flags, max_uints))
			return (0);
		(*i)++;
	}
	if (matrices[1][*i] && matrices[1][*i][0] == '-')
	{
		flags->ignore_env = 1;
		(*i)++;
	}
	return (*i);
}

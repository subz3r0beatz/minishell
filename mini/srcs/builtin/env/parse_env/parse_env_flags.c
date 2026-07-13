/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_env_flags.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 16:47:49 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/13 20:12:32 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

size_t	parse_env_flags(char ***args, char ***exported,
	t_flags *flags, size_t lens[2])
{
	size_t	*max_uints[4];
	size_t	i;

	i = 0;
	max_uints[0] = &i;
	max_uints[1] = NULL;
	max_uints[2] = &lens[0];
	max_uints[3] = &lens[1];
	while ((*args)[++i] && (*args)[i][0] == '-' && (*args)[i][1])
	{
		if ((*args)[i][1] == '-')
		{
			if (!(*args)[i][2])
				return (i + 1);
			else if (check_long_flags(args, exported, flags, max_uints))
				return (0);
			if (flags->print_help)
				return (i + 1);
		}
		else if (check_flags(args, exported, flags, max_uints))
			return (0);
	}
	if ((*args)[i] && (*args)[i][0] == '-' && !(*args)[i++][1])
		flags->ignore_env = 1;
	return (i);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_env_flags.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 16:47:49 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 21:50:04 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

size_t	parse_env_flags(char **args, t_flags *flags, char ***exported)
{
	size_t	i;

	i = 1;
	while (args[i] && args[i][0] == '-' && args[i][1])
	{
		if (args[i][1] == '-')
		{
			if (!args[i][2])
				return (i + 1);
			else if (check_long_flags(args, flags, exported, &i))
				return (0);
			if (flags->print_help)
			{
				ft_free_matrix(exported, ft_memlen(exported, sizeof(char *)));
				return (i + 1);
			}
		}
		if (check_flags(args, flags, exported, &i))
			return (0);
		i++;
	}
	if (args[i] && args[i][0] == '-' && !args[i][1])
		flags->ignore_env = 1;
	return (i);
}

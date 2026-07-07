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

static t_flags	init_flags(void)
{
	t_flags	*flags;

	flags = malloc(sizeof(flags));
	if (!flags)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	flags->print_env = 1;
	flags->print_help = 0;
	flags->ignore_env = 0;
	flags->null_term = 0;
	flags->chdir_path = NULL;
	flags->custom_argv0 = NULL;
	return (flags);
}

size_t	parse_env_flags(char **args, t_flags *flags, char **exported)
{
	size_t	i;

	flags = init_flags();
	if (!flags)
		return (0);
	i = 1;
	while (args[i] && args[i][0] == '-' && args[i][1])
	{
		if (args[i][1] == '-')
		{
			if (!args[i][2])
				return (i + 1);
			else if (check_long_flags(&args[i], flags, exported, &i))
				return (0);
		}
		if (check_flags(&args[i], flags, exported, &i))
			return (0);
		i++;
	}
	if (args[i] && args[i][0] == '-' && !args[i][1])
		flags->ignore_env = 1;
	return (i);
}

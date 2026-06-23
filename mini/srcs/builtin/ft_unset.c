/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_unset.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 07:09:11 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/23 17:46:19 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	check_flag(char *str)
{
	write(2, "minishell: unset: -", 18);
	write(2, str[1], ft_strlen(str));
	write(2, ": invalid option\n unset: usage: unset [name ...]\n", 48);
	return (0);
}

static int	is_valid_key(char *str)
{
	size_t	i;

	if (ft_isalpha(str[0]) || str[0] == '_')
	{
		i = 1;
		while (str[i])
		{
			if (!ft_isalnum(str[i]) && str[i] != '_')
				break ;
			i++;
		}
		if (str[i] == '\0')
			return (1);
	}
	write(2, "minishell: unset: '", 19);
	write(2, str, ft_strlen(str));
	write(2, "': not a valid identifier\n", 26);
	return (0);
}

static int	check_var(t_robin *env, char *str)
{
	size_t	i;

	if (!is_valid_key(str))
		return (1);
	else
		robin_remove(env, str);
	return (0);
}

int	ft_unset(t_robin *env, char **args)
{
	size_t	i;
	int		status;
	int		stop;

	if (!args[1])
		return (0);
	i = 0;
	stop = 0;
	status = 0;
	while (args[++i])
	{
		if (!stop && args[i][0] && args[i][0] == '-')
		{
			if (args[i][1] && args[i][1] == '-' && !args[i][2])
			{
				stop = 1;
				continue ;
			}
			if (args[i][1] && !check_flag(args[i]))
				return (2);
		}
		if (check_var(env, args[i]))
			status = 1;
	}
	return (status);
}

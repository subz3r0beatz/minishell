/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_env.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 03:45:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/20 21:26:17 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include "environment.h"
#include "robin_hash.h"

static int	check_flags(char **args)
{
	int	i;

	i = 0;
	while (args[++i])
	{
		
	}
	return (i);
}

static void	print_env(void *key, void *value, void *args)
{
	int		fd_out;
	t_env	*env;

	fd_out = *(int *)args;
	env = (t_env *)value;
	if (env->is_exported && env->value)
	{
		ft_putstr_fd((char *)key, fd_out);
		ft_putchar_fd('=', fd_out);
		ft_putstr_fd(env->value, fd_out);
		ft_putchar_fd('\n', fd_out);
	}
}

int	ft_env(t_robin *env, char **args, int fd_out)
{
	int	flag;

	flag = check_flags(args);
	if (flag == 2)
		return (2);
	robin_iter(env, print_env, &fd_out);
	return (0);
}

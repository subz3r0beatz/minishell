/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_env.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 03:45:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 16:56:10 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include "environment.h"
#include "robin_hash.h"

static int	check_flags(char **args, int *unset, int *ignore, int *null)
{

}

static void	print_env(void *key, void *value, void *args)
{
	int		null;	
	int		fd_out;
	t_env	*env;

	fd_out = *(int *)args[0];
	null = *(int *)args[1];
	env = (t_env *)value;
	if (env->is_exported && env->value)
	{
		ft_putstr_fd((char *)key, fd_out);
		ft_putchar_fd('=', fd_out);
		ft_putstr_fd(env->value, fd_out);
		if (null)
			ft_putchar_fd('\0', fd_out);
		else
			ft_putchar_fd('\n', fd_out);
	}
}

int	ft_env(t_robin *env, char **args, int fd_out)
{
	int	ignore;
	int	print;
	int	unset;
	int	iter_args[2];

	null = 0;
	unset = 0;
	ignore = 0;
	print = 1;
	i = check_flags(args, &unset, &ignore, &null);
	if (args[i])
		print = 0;
	if (ignore)
		return (handle_ignore(env, &args[i]));
	else if (print)
		robin_iter(env, print_env, &iter_args[0], &iter_args[1]);
	return (0);
}

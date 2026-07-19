/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exit.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/14 01:50:30 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/19 20:45:23 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	check_numeric(char *arg, int fd_out)
{
	size_t	i;

	i = 0;
	while (arg[i])
	{
		if (ft_isdigit(arg[i]) == 0)
		{
			ft_putstr_fd("exit", fd_out);
			ft_putstr_fd("minishell: exit: ", STDERR_FILENO);
			ft_putstr_fd(arg, STDERR_FILENO);
			ft_putstr_fd(": numeric argument required\n", STDERR_FILENO);
			return (0);
		}
		i++;
	}
	return (1);
}

void	ft_exit(t_minishell *shell, char **args, int fd_out)
{
	size_t	argc;
	int		status;

	argc = ft_memlen(args, sizeof(char *));
	if (argc > 2)
	{
		ft_putstr_fd("exit", fd_out);
		ft_putstr_fd("minishell: exit: too many arguments\n", STDERR_FILENO);
		status = 1;
		return ;
	}
	else if (argc <= 2)
		status = 0;
	else
		status = ft_atoi(args[1]);
	if (!check_numeric(args[1], fd_out))
		status = 2;
	close(fd_out);
	robin_free(shell->env);
	exit(status);
}

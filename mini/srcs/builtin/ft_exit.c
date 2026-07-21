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

static void	numeric_error(char *arg)
{
	if (isatty(STDERR_FILENO))
		ft_putstr_fd("exit\n", STDERR_FILENO);
	ft_putstr_fd("minishell: exit: ", STDERR_FILENO);
	ft_putstr_fd(arg, STDERR_FILENO);
	ft_putstr_fd(": numeric argument required\n", STDERR_FILENO);
}

static int	check_numeric(char *arg)
{
	size_t	i;

	i = 0;
	while (arg[i] && ft_iswhite(arg[i]))
		i++;
	if (arg[i] == '+' || arg[i] == '-')
		i++;
	if (!arg[i])
		return (1);
	while (arg[i])
	{
		if (!ft_isdigit(arg[i]))
		{
			numeric_error(arg);
			return (1);
		}
		i++;
	}
	return (0);
}

static long long	overflow_atoll(const char *str, int *error)
{
	size_t		i;
	int				sign;
	long long	nb;

	i = 0;
	sign = 1;
	while (str[i] && ft_iswhite(str[i]))
		i++;
	if (str[i] == '+' || str[i] == '-')
		if (str[i++] == '-')
			sign = -1;
	nb = 0;
	while (str[i] && ft_isdigit(str[i]))
	{
		if (nb > 922337203685477580LL || (nb == 922337203685477580LL
			&& ((sign == 1 && str[i] - '0' > 7)
			|| (sign == -1 && str[i] - '0' > 8))))
		{
			*error = 1;
			return (2);
		}
		nb = (nb * 10) + (str[i] - '0');
		i++;
	}
	return (nb * sign);
}

int	ft_exit(t_minishell *shell, char **args, int fd_out)
{
	int		status;
	int		error;
	size_t	i;

	i = 1;
	if (args[1] && args[1][0] == '-' && args[1][1] == '-' && !args[1][2])
		i++;
	error = 0;
	if (!args[i])
		status = 0;
	else if (check_numeric(args[i]))
	{
		close(fd_out);
		robin_free(shell->env);
		exit(2);
	}
	else if (args[i + 1])
	{
		if (isatty(STDERR_FILENO))
			ft_putstr_fd("exit\n", STDERR_FILENO);
		ft_putstr_fd("minishell: exit: too many arguments\n", STDERR_FILENO);
		return (1);
	}
	else
		status = overflow_atoll(args[i], &error) & 255;
	if (error)
		numeric_error(args[i]);
	else if (isatty(STDERR_FILENO))
		ft_putstr_fd("exit\n", STDERR_FILENO);
	close(fd_out);
	robin_free(shell->env);
	exit(status);
}

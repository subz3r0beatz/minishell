/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exit.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/14 01:50:30 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 20:23:50 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	numeric_error(t_minishell *shell, char *arg)
{
	if (isatty(STDERR_FILENO) && !shell->is_child)
		ft_putstr_fd("exit\n", STDERR_FILENO);
	ft_putstr_fd("minishell: exit: ", STDERR_FILENO);
	ft_putstr_fd(arg, STDERR_FILENO);
	ft_putstr_fd(": numeric argument required\n", STDERR_FILENO);
}

static int	argument_error(t_minishell *shell)
{
	if (isatty(STDERR_FILENO) && !shell->is_child)
		ft_putstr_fd("exit\n", STDERR_FILENO);
	ft_putstr_fd("minishell: exit: too many arguments\n", STDERR_FILENO);
	return (1);
}

static int	check_numeric(t_minishell *shell, char *arg)
{
	size_t	i;

	i = 0;
	while (arg[i] && ft_iswhite(arg[i]))
		i++;
	if (arg[i] == '+' || arg[i] == '-')
		i++;
	if (!arg[i] || !ft_isdigit(arg[i]))
	{
		numeric_error(shell, arg);
		return (1);
	}
	while (arg[i] && ft_isdigit(arg[i]))
		i++;
	while (arg[i] && ft_iswhite(arg[i]))
		i++;
	if (arg[i])
	{
		numeric_error(shell, arg);
		return (1);
	}
	return (0);
}

static long long	overflow_atoll(const char *str, int *error)
{
	size_t		i;
	int			sign;
	long long	nb;

	i = 0;
	sign = 1;
	while (str[i] && ft_iswhite(str[i]))
		i++;
	if (str[i] == '+' || str[i] == '-')
		if (str[i++] == '-')
			sign = -1;
	nb = 0;
	while (str[(++i) - 1] && ft_isdigit(str[i - 1]))
	{
		if (nb < -922337203685477580LL || (nb == -922337203685477580LL
				&& ((sign == 1 && str[i - 1] - '0' > 7)
					|| (sign == -1 && str[i - 1] - '0' > 8))))
			*error = 1;
		if (*error)
			return (2);
		nb = (nb * 10) - (str[i - 1] - '0');
	}
	if (sign == 1)
		return (-nb);
	return (nb);
}

int	ft_exit(t_minishell *shell, char **args)
{
	int		status;
	int		error;
	size_t	i;

	i = 1;
	if (args[1] && args[1][0] == '-' && args[1][1] == '-' && !args[1][2])
		i++;
	error = 0;
	if (!args[i])
		status = shell->exit_status;
	else if (check_numeric(shell, args[i]))
		exit_shell(shell, 2);
	else if (args[i + 1])
		return (argument_error(shell));
	else
		status = overflow_atoll(args[i], &error) & 255;
	if (error)
		numeric_error(shell, args[i]);
	else if (isatty(STDERR_FILENO) && !shell->is_child)
		ft_putstr_fd("exit\n", STDERR_FILENO);
	exit_shell(shell, status);
	return (status);
}

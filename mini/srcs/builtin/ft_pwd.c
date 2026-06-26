/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pwd.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 22:52:56 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 18:33:37 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	get_flag(char *str)
{
	int		flag;
	size_t	i;

	i = 1;
	flag = 0;
	while (str[i])
	{
		if (str[i] == 'L')
			flag = 0;
		else if (str[i] == 'P')
			flag = 1;
		else
		{
			ft_putstr_fd("minishell: pwd: -", 2);
			ft_putchar_fd(str[i], 2);
			ft_putstr_fd(": invalid option\n"
				"pwd: usage: pwd [-LP]\n", 2);
			return (2);
		}
		i++;
	}
	return (flag);
}

static int	check_flags(char **args)
{
	size_t	i;
	int		flag;

	i = 0;
	flag = 0;
	while (args[++i])
	{
		if (args[i][0] != '-' || !args[i][1])
			break ;
		if (args[i][1] == '-')
		{
			if (args[i][2])
			{
				ft_putstr_fd("minishell: pwd: --: invalid option\n"
					"pwd: usage: pwd [-LP]\n", 2);
				return (2);
			}
			break ;
		}
		flag = get_flag(args[i]);
		if (flag == 2)
			return (2);
	}
	return (flag);
}

static int	getcwd_error(void)
{
	ft_putstr_fd("minishell: pwd: "
		"error retrieving current directory: ", STDERR_FILENO);
	ft_putstr_fd("getcwd: cannot access parent directories: ", STDERR_FILENO);
	ft_putendl_fd(strerror(errno), STDERR_FILENO);
	return (1);
}

int	ft_pwd(t_robin *env, char **args, int fd_out)
{
	int				flag;
	char			pwd[8192];
	t_robin_node	*robin_node;

	flag = check_flags(args);
	if (flag == 2)
		return (2);
	if (!flag)
	{
		robin_node = robin_search(env, "PWD");
		if (robin_node && ((t_env *)robin_node->value)->value)
		{
			ft_putendl_fd(((t_env *)robin_node->value)->value, fd_out);
			return (0);
		}
	}
	if (!getcwd(pwd, 8192))
		return (getcwd_error());
	ft_putendl_fd(pwd, fd_out);
	return (0);
}

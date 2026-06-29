/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pwd.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/16 22:52:56 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/29 15:44:02 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <stddef.h>

static int	usage_error(char c)
{
	ft_putstr_fd("minishell: pwd: -", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd(": invalid option\npwd: usage: pwd [-LP]\n", STDERR_FILENO);
	return (0);
}

static int	check_flags(char **args)
{
	size_t	i;
	size_t	j;
	int		flag;

	i = 1;
	flag = 1;
	while (args[i] && args[i][0] == '-' && args[i][1] != '\0')
	{
		if (args[i][1] == '-' && !args[i][2])
			break ;
		j = 1;
		while (args[i][j])
		{
			if (args[i][j] == 'L')
				flag = 1;
			else if (args[i][j] == 'P')
				flag = 2;
			else
				return (usage_error(args[i][j]));
			j++;
		}
		i++;
	}
	return (flag);
}

static int	handle_logical(t_minishell *shell, int fd_out)
{
	t_robin_node	*robin_node;
	char			*pwd;
	struct stat		pwd_stat;
	struct stat		dot_stat;

	robin_node = robin_search(shell->env, "PWD");
	if (!robin_node)
		return (1);
	pwd = ((t_env *)robin_node->value)->value;
	if (!pwd || pwd[0] != '/')
		return (1);
	if (!stat(pwd, &pwd_stat) && !stat(".", &dot_stat))
	{
		if (pwd_stat.st_ino == dot_stat.st_ino
			&& pwd_stat.st_dev == dot_stat.st_dev)
		{
			ft_putstr_fd(pwd, fd_out);
			return (0);
		}
	}
	return (1);
}

int	ft_pwd(t_minishell *shell, char **args, int fd_out)
{
	int				flag;
	char			pwd[8192];

	flag = check_flags(args);
	if (!flag)
		return (2);
	if (flag == 1)
		if (!handle_logical(shell, fd_out))
			return (0);
	if (!getcwd(pwd, 8192))
	{
		perror("minishell: pwd: error retrieving current directory: "
			"getcwd: cannot access parent directories: ");
		return (1);
	}
	ft_putendl_fd(pwd, fd_out);
	return (0);
}

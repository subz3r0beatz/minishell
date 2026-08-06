/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 18:33:00 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 17:37:42 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	usage_error(char c)
{
	ft_putstr_fd("minishell: cd: -", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd(": invalid option\n"
		"cd: usage: cd [-L|[-P [-e]]] [-@] [dir]\n", STDERR_FILENO);
	return (0);
}

static size_t	check_flags(char **args, int *logical, int *e_flag)
{
	size_t	i;
	size_t	j;

	*logical = 1;
	*e_flag = 0;
	i = 0;
	while (args[++i] && args[i][0] == '-' && args[i][1])
	{
		if (args[i][1] == '-' && !args[i][2])
			return (i + 1);
		j = 0;
		while (args[i][++j])
		{
			if (args[i][j] == 'L')
				*logical = 1;
			else if (args[i][j] == 'P')
				*logical = 0;
			else if (args[i][j] == 'e')
				*e_flag = 1;
			else
				return (usage_error(args[i][j]));
		}
	}
	return (i);
}

int	ft_cd(t_minishell *shell, char **args)
{
	int		logical;
	int		e_flag;
	int		print_pwd;
	size_t	i;
	char	*dir;

	i = check_flags(args, &logical, &e_flag);
	if (i == 0)
		return (2);
	if (args[i] && args[i + 1])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", STDERR_FILENO);
		return (1);
	}
	if (args[i] && !args[i][0])
		return (0);
	if (parse_dir(shell, args[i], &dir, &print_pwd))
		return (1);
	if (move_dir(shell, &dir, logical, e_flag))
		return (1);
	if (print_pwd)
		ft_putendl_fd(dir, STDOUT_FILENO);
	return (update_vars(shell, dir));
}

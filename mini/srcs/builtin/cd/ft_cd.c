/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 18:33:00 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/21 23:46:24 by fldumas-         ###   ########.fr       */
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

static int	malloc_error(char *ptr1, char *ptr2)
{
	free(ptr1);
	free(ptr2);
	ft_putstr_fd("minishell: cd: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (1);
}

static int	update_vars(t_minishell *shell, char *dir)
{
	char			*pwd_value;
	char			*oldpwd_value;

	if (get_var_value(shell, "PWD", &pwd_value))
	{
		if (insert_new_node(shell, "PWD", dir, 0))
			return (malloc_error(dir, NULL));
		free(dir);
		update_var_value(shell, "OLDPWD", NULL);
	}
	else
	{
		oldpwd_value = ft_strdup(pwd_value);
		if (!oldpwd_value)
			return (malloc_error(dir, NULL));
		if (update_var_value(shell, "OLDPWD", oldpwd_value))
		{
			if (insert_new_node(shell, "OLDPWD", oldpwd_value, 0))
				return (malloc_error(dir, oldpwd_value));
			free(oldpwd_value);
		}
		update_var_value(shell, "PWD", dir);
	}
	return (0);
}

int	ft_cd(t_minishell *shell, char **args, int fd_out)
{
	int		logical;
	int		e_flag;
	int		print_path;
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
	if (parse_dir(shell, args[i], &dir, &print_path))
		return (1);
	if (move_dir(shell, &dir, logical, e_flag))
		return (1);
	if (print_path)
		ft_putendl_fd(dir, fd_out);
	if (update_vars(shell, dir))
		return (1);
	return (0);
}

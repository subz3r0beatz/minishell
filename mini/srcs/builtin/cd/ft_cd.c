/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 18:33:00 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/02 19:42:53 by fldumas-         ###   ########.fr       */
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

static int	check_flags(char **args, int *logical, int *e_flag)
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

static void	update_vars(t_minishell *shell, char *dir)
{
	t_robin_node	*pwd_node;
	t_robin_node	*oldpwd_node;
	char					*oldpwd_value;

	pwd_node = robin_search(shell->env, "PWD");
	oldpwd_node = robin_search(shell->env, "OLDPWD");
	oldpwd_value = NULL;
	if (pwd_node)
		oldpwd_value = ((t_env *)pwd_node->value)->value;
	if (oldpwd_node)
	{
		free(((t_env *)oldpwd_node->value)->value);
		((t_env *)oldpwd_node->value)->value = oldpwd_value;
	}
	else
		free(oldpwd_value);
	if (pwd_node)
		((t_env *)pwd_node->value)->value = dir;
	else
		free(dir);
}

int	ft_cd(t_minishell *shell, char **args, int fd_out)
{
	int			logical;
	int			e_flag;
	int			print_path;
	size_t	i;
	char		*dir;

	i = check_flags(args, &logical, &e_flag);
	if (i == 0)
		return (2);
	if (args[i] && args[i + 1])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", STDERR_FILENO);
		return (1);
	}
	if (parse_dir(shell, args[i], &dir, &print_path))
		return (1);
	if (move_dir(shell, &dir, logical, e_flag))
	{
		free(dir);
		return (1);
	}
	if (print_path)
		ft_putendl_fd(dir, fd_out);
	update_vars(shell, dir);
	return (0);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 18:33:00 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/02 12:29:03 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	usage_error(char c)
{
	ft_putstr_fd("minishell: cd: -", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd(": invalid option\n"
		"cd: usage: cd [-L|[-P [-e]]] [dir]\n", STDERR_FILENO);
	return (0);
}

static int	check_flags(char **args, int *logical, int *silent)
{
	size_t	i;
	size_t	j;

	i = 1;
	while (args[i] && args[i][0] == '-' && args[i][1] != '\0')
	{
		if (args[i][1] == '-' && args[i][2] == '\0')
			return (i + 1);
		j = 1;
		while (args[i][j])
		{
			if (args[i][j] == 'L')
				logical = 1;
			else if (args[i][j] == 'P')
				logical = 0;
			else if (args[i][j] == 'e')
				silent = 1;
			else
				return (usage_error(args[i][j]));
			j++;
		}
		i++;
	}
	return (i);
}

static int	parse_dir(t_minishell *shell, char **args, int logical)
{
	size_t			i;
	t_robin_node	*robin_node;
	char			*file_path;

	i = 0;
	if (!args[i])
	{
		robin_node = robin_search(shell->env, "HOME");
		if (!robin_node)
		{
			ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
			return (1);
		}
		file_path = ((t_env *)robin_node->value)->value;
		if (!chdir(file_path))
			return (0);
		ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
		perror(file_path);
		return (1);
	}
	if (args[i + 1])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", STDERR_FILENO);
		return (1);
	}
	return (0);
}

int	ft_cd(t_minishell *shell, char **args, int fd_out)
{
	size_t	i;
	int		logical;
	int		silent;

	logical = 1;
	silent = 0;
	i = check_flags(args, &logical, &silent);
	if (i == 0)
		return (2);
	if (parse_dir(shell, &args[i], logical))
		return (!silent);
	return (0);
}

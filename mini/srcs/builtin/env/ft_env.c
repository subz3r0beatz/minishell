/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_env.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 03:45:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/29 22:56:15 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	print_env(char **exported, int null, int fd_out)
{
	size_t	i;
	size_t	j;

	i = 0;
	while (exported[i])
	{
		j = 0;
		while (exported[i][j] && exported[i][j] != '=')
			j++;
		write(fd_out, exported[i], j);
		if (exported[i][j])
		{
			write(fd_out, "=", 1);
			write(fd_out, exported[i] + j + 1, ft_strlen(exported[i] + j + 1));
		}
		if (null)
			write(fd_out, "\0", 1);
		else
			write(fd_out, "\n", 1);
		i++;
	}
}

static int	usage_error(char c)
{
	ft_putstr_fd("minishell: env: invalid option -- '", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd("'\n"
		"Try 'env --help' for more information.\n", STDERR_FILENO);
	return (0);
}

static int	is_invalid_key(char *str)
{
	size_t	i;

	if (!str)
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'u'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	i = 0;
	while (str[i])
	{
		if (str[i] == '=')
		{
			ft_putstr_fd("minishell: env: cannot unset '", STDERR_FILENO);
			ft_putstr_fd(str, STDERR_FILENO);
			ft_putstr_fd("': Invalid argument\n", STDERR_FILENO);
			return (1);
		}
		i++;
	}
	return (0);
}

static void	shift_down(char **exported, size_t i)
{
	while (exported[i])
	{
		exported[i] = exported[i + 1];
		i++;
	}
	exported[i] = NULL;
}

static int	handle_unset(char **exported, char **args, size_t *i, size_t j)
{
	char	*str;
	size_t	len;

	if (args[*i][j])
		str = ft_strdup(&args[*i][j]);
	else
		str = ft_strdup(args[++*i]);
	if (!str)
		return (1);
	if (is_invalid_key(str))
		return (1);
	len = ft_strlen(str);
	j = 0;
	while (exported[j]
		&& (ft_strncmp(exported[j], str, len) || exported[j][len] != '='))
		j++;
	free(str);
	if (!exported[j])
		return (0);
	free(exported[j]);
	shift_down(exported, j);
	return (0);
}

static int	parse_flags(char **args, char **exported, int *null)
{
	size_t	i;
	size_t	j;

	i = 1;
	while (args[i] && args[i][0] == '-')
	{
		if (args[i][1] == '-' && !args[i][2])
			return (i + 1);
		j = 1;
		while (args[i][j])
		{
			if (args[i][j] == 'u' && handle_unset(exported, args, &i, j + 1))
				return (0);
			else if (args[i][j] == '0')
				*null = 1;
			else if (args[i][j] == 'S' && handle_string(exported, &args[i]))

			else
				return (usage_error(args[i][j]));
			j++;
		}
		i++;
	}
}

int	ft_env(t_minishell *shell, char **args, int fd_out)
{
	size_t	i;
	int		null;
	char	**exported;

	exported = env_to_matrix(minishell->env);
	if (!exported)
		return (125);
	null = 0;
	i = parse_flags(args, exported, null);
	if (i == 0)
		return (125);
	if (!args[i])
		return (print_env(exported, null, fd_out));
	if (null)
	{
		ft_putstr_fd("minishell: env: "
			"cannot specify --null (-0) with command\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (0);
	}
}

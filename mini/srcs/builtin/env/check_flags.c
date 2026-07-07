/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_flags.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 18:21:24 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 21:50:00 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	flag_error(char c)
{
	ft_putstr_fd("minishell: env: invalid option -- '", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd("'\nTry 'env --help' for more information.\n",
		STDERR_FILENO);
	return (1);
}

static int	parse_chdir_path(char **args, t_flags *flags, size_t *i, size_t j)
{
	if (!args[*i][j + 1] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'C'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if (args[*i][j + 1])
		flags->chdir_path = ft_strdup(&args[*i][j + 1]);
	else
		flags->chdir_path = ft_strdup(args[++*i]);
	if (!flags->chdir_path)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	return (0);
}

static int	parse_argv0(char **args, t_flags *flags, size_t *i, size_t j)
{
	if (!args[*i][j + 1] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'a'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if (args[*i][j + 1])
		flags->custom_argv0 = ft_strdup(&args[*i][j + 1]);
	else
		flags->custom_argv0 = ft_strdup(args[++*i]);
	if (!flags->custom_argv0)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	return (0);
}

static int	unset_env(char **exported, char *unset)
{
	size_t	i;

	i = 0;
	while (exported[i])
	{
		if (ft_strncmp(exported[i], unset, ft_strlen(unset)) == 0)
		{
			free(exported[i]);
			ft_mem_shift(&exported[i], 1, sizeof(char *), -1);
			exported = ft_realloc(exported,
					ft_memlen(exported, sizeof(char *)), sizeof(char *));
			if (!exported)
			{
				ft_putstr_fd("minishell: env: malloc: "
					"cannot allocate memory\n", STDERR_FILENO);
				return (1);
			}
			return (0);
		}
		i++;
	}
	return (0);
}

static int	handle_unset(char **args, char **exported, size_t *i, size_t j)
{
	char	*unset;

	if (!args[*i][j + 1] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'u'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if (args[*i][j + 1])
		unset = args[*i][j + 1];
	else
		unset = args[++*i];
	if (!*unset || ft_strchr(unset, '='))
	{
		ft_putstr_fd("minishell: env: cannot unset '", STDERR_FILENO);
		ft_putstr_fd(unset, STDERR_FILENO);
		ft_putstr_fd("': Invalid argument\n", STDERR_FILENO);
		return (1);
	}
	if (unset_env(exported, unset))
		return (1);
	return (0);
}

static int	handle_string(char **args, size_t *i, size_t j)
{
	char	*string;

	if (!args[*i][j + 1] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'S'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if (args[*i][j + 1])
		string = args[*i][j + 1];
	else
		string = args[++*i];
	if (parse_string(args, string))
		return (1);
	return (0);
}

static int	check_flags(char **args, t_flags *flags,
	char **exported, size_t *i)
{
	int		ret;
	size_t	j;

	ret = 0;
	j = 0;
	while (args[*i][++j])
	{
		if (args[*i][j] == 'i')
			flags->ignore_env = 1;
		else if (args[*i][j] == '0')
			flags->null_term = 1;
		else if (args[*i][j] == 'C')
			ret = parse_chdir_path(args, flags, i, j);
		else if (args[*i][j] == 'a')
			ret = parse_argv0(args, flags, i, j);
		else if (args[*i][j] == 'u')
			ret = handle_unset(args[*i], exported, i, j);
		else if (args[*i][j] == 'S')
			ret = handle_string(args, i, j);
		else
			return (flag_error(args[*i][j]));
		if (ret)
			return (0);
	}
	return (0);
}

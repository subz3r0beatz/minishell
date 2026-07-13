/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_env.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 03:45:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/13 21:09:36 by fldumas-         ###   ########.fr       */
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
			write(fd_out, &exported[i][j + 1], ft_strlen(&exported[i][j + 1]));
		}
		if (null)
			write(fd_out, "\0", 1);
		else
			write(fd_out, "\n", 1);
		i++;
	}
}

static int	exit_env(char ***matrices[2], t_flags *flags,
	size_t lens[2], int exit_code)
{
	if (!matrices)
		return (exit_code);
	if (matrices[0] && *matrices[0])
		ft_free_matrix(*matrices[0], lens[1]);
	if (matrices[1] && *matrices[1])
		ft_free_matrix(*matrices[1], lens[0]);
	if (!flags)
		return (exit_code);
	if (flags->custom_argv0)
		free(flags->custom_argv0);
	if (flags->chdir_path)
		free(flags->chdir_path);
	return (exit_code);
}

int	ft_env(t_minishell *shell, char **args, int fd_out)
{
	char	***matrices[2];
	char	**exported;
	char	**tmp;
	char	**args_cpy;
	t_flags	flags;
	size_t	i;
	size_t	capacity;
	size_t	lens[2];
	size_t	*max_uints[2];

	exported = env_to_matrix(shell);
	if (!exported)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (exit_env(NULL, NULL, 0, 125));
	}
	matrices[0] = NULL;
	matrices[1] = &exported;
	lens[0] = ft_memlen(exported, sizeof(char *));
	lens[1] = ft_memlen(args, sizeof(char *));
	args_cpy = ft_dup_matrix(args, lens[1]);
	if (!args_cpy)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (exit_env(matrices, NULL, lens, 125));
	}
	matrices[0] = &args_cpy;
	ft_bzero(&flags, sizeof(t_flags));
	i = parse_env_flags(&args_cpy, &exported, &flags, lens);
	if (flags.print_help)
		return (exit_env(matrices, &flags, lens, print_env_help(fd_out)));
	if (i == 0)
		return (exit_env(matrices, &flags, lens, 125));
	capacity = shell->exported_count;
	if (flags.ignore_env)
	{
		ft_free_matrix(exported, lens[0]);
		exported = malloc(1 * sizeof(char *));
		if (!exported)
		{
			ft_putstr_fd("minishell: env: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			return (exit_env(matrices, &flags, lens, 125));
		}
		exported[0] = NULL;
		lens[0] = 0;
		capacity = 0;
	}
	max_uints[0] = &i;
	max_uints[1] = &lens[0];
	i = add_variables(args_cpy, &exported, max_uints, &capacity);
	if (i == 0)
		return (exit_env(matrices, &flags, lens, 125));
	if (args_cpy[i])
	{
		if (flags.null_term)
		{
			ft_putstr_fd("minishell: env: cannot specify --null (-0) with comm"
				"and\nTry 'env --help' for more information.\n", STDERR_FILENO);
			return (exit_env(matrices, &flags, lens, 125));
		}
	}
	else
	{
		if (flags.chdir_path)
		{
			ft_putstr_fd("minishell: env: must specify command with --chdir (-C"
				")\nTry 'env --help' for more information.\n", STDERR_FILENO);
			return (exit_env(matrices, &flags, lens, 125));
		}
		else if (flags.custom_argv0)
		{
			ft_putstr_fd("minishell: env: must specify command with --argv0 (-a"
				")\nTry 'env --help' for more information.\n", STDERR_FILENO);
			return (exit_env(matrices, &flags, lens, 125));
		}
		print_env(exported, flags.null_term, fd_out);
		return (exit_env(matrices, &flags, lens, 0));
	}
	tmp = ft_realloc(exported, lens[0] + 1, sizeof(char *));
	if (!tmp)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (exit_env(matrices, &flags, lens, 125));
	}
	exported = tmp;
	return (exit_env(matrices, &flags, lens,
			exec_env(&args_cpy[i], exported, &flags, lens)));
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_env.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 03:45:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 17:03:45 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	init_env(t_minishell *shell, char **matrices[2],
		t_flags *flags, t_max_uints *max_uints)
{
	ft_bzero(flags, sizeof(t_flags));
	ft_bzero(max_uints, sizeof(t_max_uints));
	matrices[0] = env_to_matrix(shell);
	if (!matrices[0])
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (exit_env(NULL, NULL, NULL, 125));
	}
	max_uints->exported_len = ft_memlen(matrices[0], sizeof(char *));
	max_uints->args_len = ft_memlen(matrices[1], sizeof(char *));
	matrices[1] = ft_dup_matrix(matrices[1], max_uints->args_len);
	if (!matrices[1])
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (exit_env(matrices, NULL, max_uints, 125));
	}
	max_uints->capacity = shell->exported_count;
	return (0);
}

static int	redo_env(char **matrices[2],
		t_flags *flags, t_max_uints *max_uints, int *status)
{
	if (flags->ignore_env)
	{
		ft_free_matrix(matrices[0], max_uints->exported_len);
		matrices[0] = malloc(1 * sizeof(char *));
		if (!matrices[0])
		{
			ft_putstr_fd("minishell: env: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			*status = 125;
			return (exit_env(matrices, flags, max_uints, 125));
		}
		matrices[0][0] = NULL;
		max_uints->exported_len = 0;
		max_uints->capacity = 0;
	}
	max_uints->i = add_variables(matrices, max_uints);
	if (max_uints->i == 0)
	{
		*status = 125;
		return (exit_env(matrices, flags, max_uints, 125));
	}
	return (0);
}

static int	check_flags(char **matrices[2],
		t_flags *flags, t_max_uints *max_uints, int fd_out)
{
	if (!matrices[1][max_uints->i])
	{
		if (flags->chdir_path || flags->custom_argv0)
		{
			ft_putstr_fd("minishell: env: must specify command", STDERR_FILENO);
			if (flags->chdir_path)
				ft_putstr_fd(" with --chdir (-C)\n", STDERR_FILENO);
			else
				ft_putstr_fd(" with --argv0 (-a)\n", STDERR_FILENO);
			ft_putstr_fd("Try 'env --help' for more information.\n",
				STDERR_FILENO);
			return (exit_env(matrices, flags, max_uints, 126));
		}
		print_env(matrices[0], flags->null_term, fd_out);
		return (exit_env(matrices, flags, max_uints, 1));
	}
	if (flags->null_term)
	{
		ft_putstr_fd("minishell: env: cannot specify --null (-0) with comm"
			"and\nTry 'env --help' for more information.\n", STDERR_FILENO);
		return (exit_env(matrices, flags, max_uints, 126));
	}
	return (0);
}

static int	resize_env(char **matrices[2],
		t_flags *flags, t_max_uints *max_uints, int *status)
{
	char	**tmp;

	if (max_uints->exported_len < max_uints->capacity)
	{
		tmp = ft_realloc(matrices[0],
				max_uints->exported_len + 1, sizeof(char *));
		if (!tmp)
		{
			ft_putstr_fd("minishell: env: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			*status = 125;
			return (exit_env(matrices, flags, max_uints, 125));
		}
		matrices[0] = tmp;
		max_uints->capacity = max_uints->exported_len;
	}
	return (0);
}

int	ft_env(t_minishell *shell, char **args, int fd_out)
{
	int			status;
	char		**matrices[2];
	t_max_uints	max_uints;
	t_flags		flags;

	matrices[1] = args;
	status = init_env(shell, matrices, &flags, &max_uints);
	if (status)
		return (status);
	max_uints.i = parse_env_flags(matrices, &flags, &max_uints);
	if (flags.print_help)
		return (exit_env(matrices, &flags, &max_uints, print_env_help(fd_out)));
	if (max_uints.i == 0)
		return (exit_env(matrices, &flags, &max_uints, 125));
	if (redo_env(matrices, &flags, &max_uints, &status))
		return (status);
	status = check_flags(matrices, &flags, &max_uints, fd_out);
	if (status)
		return (status - 1);
	if (resize_env(matrices, &flags, &max_uints, &status))
		return (status);
	return (exit_env(matrices, &flags, &max_uints,
			exec_env(matrices, &flags, &max_uints)));
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_env.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 03:45:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 21:49:56 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <stddef.h>

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

static int	exit_env(char **args, char **exported,
	size_t lens[2], int exit_code)
{
	if (args)
		ft_free_matrix(args, lens[1]);
	if (exported)
		ft_free_matrix(exported, lens[0]));
	return (exit_code);
}

int	ft_env(t_minishell *shell, char **args, int fd_out)
{
	char	**exported;
	char	**args_cpy;
	t_flags	flags;
	size_t	i;
	size_t	lens[2];

	exported = env_to_matrix(shell);
	if (!exported)
		return (exit_env(NULL, NULL, 0, 125));
	ft_bzero(flags, sizeof(t_flags));
	lens[0] = ft_memlen(exported, sizeof(char *));
	lens[1] = ft_memlen(args, sizeof(char *));
	args_cpy = ft_dup_matrix(args, lens[1]);
	if (!args_cpy)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (exit_env(NULL, exported, lens, 125));
	}
	i = parse_env_flags(&args_cpy, &exported, &flags, &lens);
	if (flags.print_help)
		return (exit_env(args_cpy, exported, lens,  print_env_help(fd_out));
	if (i == 0)
		return (exit_env(args_cpy, exported, lens, 125));
	i = add_variables(args_cpy, &exported, i, &lens[0]);
	if (i == 0)
		return (exit_env(args_cpy, exported, lens, 125));
	if (flags.ignore_env)
	{
		ft_free_matrix(exported, lens[0]);
		exported = malloc(1 * sizeof(char *));
		if (!exported)
		{
			ft_putstr_fd("minishell: env: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			exit_env(args_cpy, NULL, lens, 125);
		}
		exported[0] = NULL;
		lens[0] = 0;
	}
	if (args[i])
	{
		if (flags.null_term)
		{
			ft_putstr_fd("minishell: env: cannot specify --null (-0) with command\n"
				"Try 'env --help' for more information.\n", STDERR_FILENO);
			return (exit_env(args_cpy, exported, lens, 125));
		}
	}
	else
		print_env(exported, flags.null_term, fd_out);
	return (exit_env(args_cpy, exported, lens, 0));
}

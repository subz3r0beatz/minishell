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

static size_t	add_variables(char **args, char **exported, size_t i)
{
	char	*ptr;
	char	*key;

	while (args[i])
	{
		ptr = ft_strchr(args[i], '=');
		if (ptr)
		{
			key = ft_substr(args[i], 0, ptr - args[i]);
			if (!key)
			{
				ft_putstr_fd("minishell: env: malloc: "
					"cannot allocate memory\n", STDERR_FILENO);
				return (0);
			}
		}
		else
			break ;
		i++;
	}
	return (i);
}

int	ft_env(t_minishell *shell, char **args, int fd_out)
{
	char	**exported;
	t_flags	*flags;
	size_t	i;

	exported = env_to_matrix(shell);
	if (!exported)
		return (125);
	i = parse_env_flags(args, flags, exported);
	if (flags->print_help)
		return (print_env_help());
	if (i == 0)
	{
		ft_free_matrix(exported, ft_memlen(exported, sizeof(char *)));
		return (125);
	}
	i = add_variables(&args[i], exported, shell);
	if (i == 0)
		return (125);
	if (args[i])
		flags->print_env = 0;
	if (flags->print_env)
		print_env(exported, flags->null_term, fd_out);
	ft_free_matrix(exported, ft_memlen(exported, sizeof(char *)));
	return (0);
}

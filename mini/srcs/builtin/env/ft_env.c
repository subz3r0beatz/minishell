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

static t_flags	*init_flags(void)
{
	t_flags	*flags;

	flags = malloc(sizeof(t_flags));
	if (!flags)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	flags->print_env = 1;
	flags->print_help = 0;
	flags->ignore_env = 0;
	flags->null_term = 0;
	flags->chdir_path = NULL;
	flags->custom_argv0 = NULL;
	return (flags);
}

int	ft_env(t_minishell *shell, char **args, int fd_out)
{
	char	**exported;
	t_flags	*flags;
	size_t	i;

	exported = env_to_matrix(shell);
	if (!exported)
		return (125);
	flags = init_flags();
	if (!flags)
		return (125);
	i = parse_env_flags(args, flags, exported);
	if (flags->print_help)
		return (print_env_help());
	if (i == 0)
		return (125);
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

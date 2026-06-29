/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 06:51:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 21:20:26 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	usage_error(char c)
{
	ft_putstr_fd("minishell: export: -", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd(": invalid option\nexport: usage: export"
		" [-n] [name[=value] ...] or export -p\n", STDERR_FILENO);
	return (0);
}

static int	check_flags(char **args, int *print, int *unexport)
{
	size_t	i;
	size_t	j;

	i = 0;
	while (args[++i] && args[i][0] == '-' && args[i][1] != '\0')
	{
		if (args[i][1] == '-' && args[i][2] == '\0')
		{
			i++;
			break ;
		}
		j = 0;
		while (args[i][++j])
			if (!ft_strchr("pn", args[i][j]))
				return (usage_error(args[i][j]));
		j = 0;
		while (args[i][++j])
			if (args[i][j] == 'n')
				*unexport = 1;
	}
	if (args[i])
		*print = 0;
	return (i);
}

int	ft_export(t_minishell *minishell, char **args, int fd_out)
{
	int		status;
	int		print;
	int		unexport;
	size_t	i;
	t_robin	*env;

	env = minishell->env;
	print = 1;
	unexport = 0;
	i = check_flags(args, &print, &unexport);
	if (i == 0)
		return (1);
	if (print)
		return (print_export(env, fd_out));
	else if (unexport)
		status = unexport_vars(env, &args[i]);
	else
		status = parse_vars(env, &args[i]);
	free_matrix(minishell->exported,
		ft_memlen(minishell->exported, sizeof(char *)));
	minishell->exported = NULL;
	return (status);
}

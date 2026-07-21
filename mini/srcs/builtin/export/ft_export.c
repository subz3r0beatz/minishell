/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 06:51:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 01:05:30 by fldumas-         ###   ########.fr       */
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

static size_t	check_flags(char **args, int *print, int *unexport)
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
		{
			if (args[i][j] == 'n')
				*unexport = 1;
			else if (args[i][j] == 'p')
				*print = 1;
			else
				return (usage_error(args[i][j]));
		}
	}
	if (args[i])
		*print = 0;
	return (i);
}

int	ft_export(t_minishell *shell, char **args, int fd_out)
{
	int		status;
	int		print;
	int		unexport;
	size_t	i;

	print = 1;
	unexport = 0;
	i = check_flags(args, &print, &unexport);
	if (i == 0)
		return (2);
	if (print)
		return (print_export(shell->env, fd_out));
	else if (unexport)
		status = unexport_vars(shell, &args[i]);
	else
		status = parse_export_vars(shell, &args[i]);
	ft_free_matrix(shell->exported,
		ft_memlen(shell->exported, sizeof(char *)));
	shell->exported = NULL;
	return (status);
}

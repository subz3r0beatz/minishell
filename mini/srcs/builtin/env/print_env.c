/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 14:38:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:28:15 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	print_env(char **exported, int null)
{
	size_t	i;
	size_t	j;

	i = 0;
	while (exported[i])
	{
		j = 0;
		while (exported[i][j] && exported[i][j] != '=')
			j++;
		write(STDOUT_FILENO, exported[i], j);
		if (exported[i][j])
		{
			write(STDOUT_FILENO, "=", 1);
			write(STDOUT_FILENO, &exported[i][j + 1],
				ft_strlen(&exported[i][j + 1]));
		}
		if (null)
			write(STDOUT_FILENO, "\0", 1);
		else
			write(STDOUT_FILENO, "\n", 1);
		i++;
	}
}

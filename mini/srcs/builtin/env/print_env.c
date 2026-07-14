/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 14:38:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 14:39:08 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	print_env(char **exported, int null, int fd_out)
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

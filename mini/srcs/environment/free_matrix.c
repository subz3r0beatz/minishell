/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_matrix.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 20:44:31 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 20:45:25 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	free_matrix(char **matrix, size_t size)
{
	size_t	i;

	i = 0;
	while (i < size)
	{
		free(matrix[i]);
		i++;
	}
	free(matrix);
}

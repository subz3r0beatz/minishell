/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_dup_matrix.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 14:57:37 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 00:16:23 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	**ft_dup_matrix(char **matrix, size_t size)
{
	char	**dup;
	size_t	i;

	i = 0;
	dup = malloc((size + 1) * sizeof(char *));
	if (!dup)
		return (NULL);
	while (i < size)
	{
		dup[i] = ft_strdup(matrix[i]);
		if (!dup[i])
		{
			ft_free_matrix(dup, i);
			return (NULL);
		}
		i++;
	}
	dup[i] = NULL;
	return (dup);
}

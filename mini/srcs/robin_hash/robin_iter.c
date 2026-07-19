/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_iter.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 02:24:38 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/19 16:04:08 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	robin_iter(t_robin *robin,
		int (*f)(const void *key, void *value, void *args), void *args)
{
	size_t	i;

	i = 0;
	while (i < robin->capacity)
	{
		if (robin->ctrl[i] > 0)
			if (f(robin->data[i].key, robin->data[i].value, args))
				return (1);
		i++;
	}
	return (0);
}

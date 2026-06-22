/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_iter.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 02:24:38 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/11 16:55:28 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "robin_hash.h"

void	robin_iter(t_robin *robin,
		void (*f)(const void *key, void *value, void *args), void *args)
{
	size_t	i;

	i = 0;
	while (i < robin->capacity)
	{
		if (robin->ctrl[i] > 0)
			f(robin->data[i].key, robin->data[i].value, args);
		i++;
	}
}

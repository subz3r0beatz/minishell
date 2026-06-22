/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_search.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 21:13:34 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/11 17:42:22 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "robin_hash.h"

t_robin_node	*robin_search(const t_robin *robin, const void *key)
{
	size_t	target_hash;
	size_t	hash;
	uint8_t	psl;
	size_t	i;

	if (robin->count == 0)
		return (NULL);
	target_hash = robin->hash_function(key);
	hash = target_hash & (robin->capacity - 1);
	psl = 0;
	i = 0;
	while (i < robin->capacity)
	{
		if (robin->ctrl[hash] == 0 || psl > robin->ctrl[hash] - 1)
			return (NULL);
		if (robin->data[hash].hash == target_hash
			&& robin->cmp_function(robin->data[hash].key, key) == 0)
			return (&robin->data[hash]);
		hash = (hash + 1) & (robin->capacity - 1);
		psl++;
		i++;
	}
	return (NULL);
}

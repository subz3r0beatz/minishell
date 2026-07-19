/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_search.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 21:13:34 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/19 16:04:29 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <stddef.h>

t_robin_node	*robin_search(const t_robin *robin, const void *key)
{
	size_t	mask;
	size_t	target_hash;
	size_t	hash;
	size_t	i;
	uint8_t	psl;

	if (robin->count == 0)
		return (NULL);
	target_hash = robin->hash_function(key);
	mask = robin->capacity - 1;
	hash = target_hash & mask;
	psl = 0;
	i = 0;
	while (i < mask + 1)
	{
		if (robin->ctrl[hash] == 0 || psl > robin->ctrl[hash] - 1)
			return (NULL);
		if (robin->data[hash].hash == target_hash
			&& robin->cmp_function(robin->data[hash].key, key) == 0)
			return (&robin->data[hash]);
		hash = (hash + 1) & mask;
		psl++;
		i++;
	}
	return (NULL);
}

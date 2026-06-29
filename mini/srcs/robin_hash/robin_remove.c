/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_remove.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 22:03:15 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/11 17:42:18 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	get_index(t_robin *robin, const void *key,
		size_t *hash, size_t target_hash)
{
	uint8_t	psl;
	size_t	i;

	psl = 0;
	i = 0;
	while (i < robin->capacity)
	{
		if (robin->ctrl[*hash] == 0 || psl > robin->ctrl[*hash] - 1)
			return (1);
		if (robin->data[*hash].hash == target_hash
			&& robin->cmp_function(robin->data[*hash].key, key) == 0)
			return (0);
		*hash = (*hash + 1) & (robin->capacity - 1);
		psl++;
		i++;
	}
	return (1);
}

int	robin_remove(t_robin *robin, const void *key)
{
	size_t	target_hash;
	size_t	hash;
	size_t	next_hash;

	if (!robin || robin->count == 0)
		return (1);
	target_hash = robin->hash_function(key);
	hash = target_hash & (robin->capacity - 1);
	if (get_index(robin, key, &hash, target_hash))
		return (1);
	robin->del_function(robin->data[hash].key, robin->data[hash].value);
	while (1)
	{
		next_hash = (hash + 1) & (robin->capacity - 1);
		if (robin->ctrl[next_hash] == 0 || robin->ctrl[next_hash] == 1)
		{
			robin->ctrl[hash] = 0;
			robin->count--;
			return (0);
		}
		robin->data[hash] = robin->data[next_hash];
		robin->ctrl[hash] = robin->ctrl[next_hash] - 1;
		hash = next_hash;
	}
	return (1);
}

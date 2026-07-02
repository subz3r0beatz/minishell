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
	size_t	mask;

	mask = robin->capacity - 1;
	psl = 0;
	i = 0;
	while (i < mask + 1)
	{
		if (robin->ctrl[*hash] == 0 || psl > robin->ctrl[*hash] - 1)
			return (1);
		if (robin->data[*hash].hash == target_hash
			&& robin->cmp_function(robin->data[*hash].key, key) == 0)
			return (0);
		*hash = (*hash + 1) & mask;
		psl++;
		i++;
	}
	return (1);
}

int	robin_remove(t_robin *robin, const void *key)
{
	size_t	mask;
	size_t	target_hash;
	size_t	hash;
	size_t	next_hash;

	if (!robin || robin->count == 0)
		return (1);
	target_hash = robin->hash_function(key);
	mask = robin->capacity - 1;
	hash = target_hash & mask;
	if (get_index(robin, key, &hash, target_hash))
		return (1);
	robin->del_function(robin->data[hash].key, robin->data[hash].value);
	while (1)
	{
		next_hash = (hash + 1) & mask;
		if (robin->ctrl[next_hash] == 0 || robin->ctrl[next_hash] == 1)
			break ;
		robin->data[hash] = robin->data[next_hash];
		robin->ctrl[hash] = robin->ctrl[next_hash] - 1;
		hash = next_hash;
	}
	robin->ctrl[hash] = 0;
	robin->count--;
	return (0);
}

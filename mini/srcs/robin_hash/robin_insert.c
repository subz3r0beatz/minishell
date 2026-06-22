/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_insert.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:44:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/11 17:27:37 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "robin_hash.h"

static int	empty_node(t_robin *robin, size_t hash,
		t_robin_node node, uint8_t psl)
{
	if (robin->ctrl[hash] == 0)
	{
		robin->data[hash] = node;
		robin->ctrl[hash] = psl + 1;
		robin->count++;
		return (1);
	}
	return (0);
}

static int	key_match(t_robin *robin, t_robin_node node, size_t hash)
{
	if (robin->data[hash].hash == node.hash
		&& robin->cmp_function(robin->data[hash].key, node.key) == 0)
	{
		robin->del_function(robin->data[hash].key, robin->data[hash].value);
		robin->data[hash] = node;
		return (1);
	}
	return (0);
}

static void	swap_node(t_robin *robin, size_t hash,
		t_robin_node *node, uint8_t *psl)
{
	t_robin_node	tmp_node;
	uint8_t			tmp_psl;

	tmp_node = robin->data[hash];
	robin->data[hash] = *node;
	*node = tmp_node;
	tmp_psl = robin->ctrl[hash] - 1;
	robin->ctrl[hash] = *psl + 1;
	*psl = tmp_psl;
}

void	robin_insert(t_robin *robin, t_robin_node node)
{
	size_t			hash;
	uint8_t			psl;

	if (robin->count >= (robin->capacity * 85) / 100)
		if (robin_expand(robin))
			return ;
	node.hash = robin->hash_function(node.key);
	hash = node.hash & (robin->capacity - 1);
	psl = 0;
	while (psl < 254)
	{
		if (empty_node(robin, hash, node, psl))
			return ;
		if (key_match(robin, node, hash))
			return ;
		if (psl > robin->ctrl[hash] - 1)
			swap_node(robin, hash, &node, &psl);
		hash = (hash + 1) & (robin->capacity - 1);
		psl++;
	}
	if (robin_expand(robin) == 0)
		robin_insert(robin, node);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_insert.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:44:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 03:39:48 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	empty_node(t_robin *robin, size_t hash,
		t_robin_node node, uint8_t psl)
{
	if (robin->ctrl[hash] == 0)
	{
		robin->data[hash] = node;
		robin->ctrl[hash] = psl + 1;
		robin->count++;
		return (0);
	}
	return (1);
}

static int	key_match(t_robin *robin, t_robin_node node, size_t hash)
{
	if (robin->data[hash].hash == node.hash
		&& robin->cmp_function(robin->data[hash].key, node.key) == 0)
	{
		robin->del_function(robin->data[hash].key, robin->data[hash].value);
		robin->data[hash] = node;
		return (0);
	}
	return (1);
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

int	robin_insert(t_robin *robin, t_robin_node node)
{
	size_t	mask;
	size_t	hash;
	uint8_t	psl;

	if (robin->count >= (robin->capacity * 85) / 100)
		if (robin_expand(robin))
			return (1);
	node.hash = robin->hash_function(node.key);
	mask = robin->capacity - 1;
	hash = node.hash & mask;
	psl = 0;
	while (psl < 255)
	{
		if (!empty_node(robin, hash, node, psl)
			|| !key_match(robin, node, hash))
			return (0);
		if (psl > robin->ctrl[hash] - 1)
			swap_node(robin, hash, &node, &psl);
		hash = (hash + 1) & mask;
		psl++;
	}
	if (robin_expand(robin) == 0)
		return (robin_insert(robin, node));
	else
		return (1);
}

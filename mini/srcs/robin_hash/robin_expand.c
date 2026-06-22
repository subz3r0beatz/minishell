/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_expand.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 18:35:19 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/11 17:23:56 by fldumas-         ###   ########.fr       */
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

static void	fast_robin_insert(t_robin *robin, t_robin_node node)
{
	size_t			hash;
	uint8_t			psl;

	hash = node.hash & (robin->capacity - 1);
	psl = 0;
	while (psl < 254)
	{
		if (empty_node(robin, hash, node, psl))
			return ;
		if (psl > robin->ctrl[hash] - 1)
			swap_node(robin, hash, &node, &psl);
		hash = (hash + 1) & (robin->capacity - 1);
		psl++;
	}
	if (robin_expand(robin) == 0)
		fast_robin_insert(robin, node);
}

int	robin_expand(t_robin *robin)
{
	size_t	i;
	t_robin	*new_robin;

	if (robin->capacity >= SIZE_MAX / 2)
		return (1);
	new_robin = robin_init(robin->capacity * 2,
			robin->hash_function, robin->cmp_function, robin->del_function);
	if (!new_robin)
		return (1);
	i = 0;
	while (i < robin->capacity)
	{
		if (robin->ctrl[i])
			fast_robin_insert(new_robin, robin->data[i]);
		i++;
	}
	free(robin->ctrl);
	free(robin->data);
	robin->ctrl = new_robin->ctrl;
	robin->data = new_robin->data;
	robin->capacity = new_robin->capacity;
	robin->count = new_robin->count;
	free(new_robin);
	return (0);
}

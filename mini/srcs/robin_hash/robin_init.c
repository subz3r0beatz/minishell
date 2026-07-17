/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_init.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:49:13 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/17 03:31:38 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_robin	*robin_init(const size_t capacity,
		size_t (*hash_function)(const void *key),
		int (*cmp_function)(const void *key, const void *new_key),
		int (*del_function)(void *key, void *value))
{
	t_robin	*robin;

	if (!(capacity && !(capacity & (capacity - 1)))
		|| !hash_function || !cmp_function || !del_function)
		return (NULL);
	robin = malloc(sizeof(t_robin));
	if (!robin)
		return (NULL);
	robin->ctrl = ft_calloc(capacity, sizeof(uint8_t));
	robin->data = malloc(capacity * sizeof(t_robin_node));
	if (!robin->ctrl || !robin->data)
	{
		free(robin->ctrl);
		free(robin->data);
		free(robin);
		return (NULL);
	}
	robin->capacity = capacity;
	robin->count = 0;
	robin->hash_function = hash_function;
	robin->cmp_function = cmp_function;
	robin->del_function = del_function;
	return (robin);
}

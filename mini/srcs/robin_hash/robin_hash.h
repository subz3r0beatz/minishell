/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_hash.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 15:20:59 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 13:34:02 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROBIN_HASH_H
# define ROBIN_HASH_H

# include <stdlib.h>
# include <stdint.h>

# include "hash_functions/hash_functions.h"

typedef struct s_robin_node
{
	void	*key;
	void	*value;
	size_t	hash;
}				t_robin_node;

typedef struct s_robin
{
	uint8_t			*ctrl;
	t_robin_node	*data;
	size_t			capacity;
	size_t			count;
	size_t			(*hash_function)(const void *key);
	int				(*cmp_function)(const void *key, const void *new_key);
	void			(*del_function)(void *key, void *value);
}				t_robin;

t_robin			*robin_init(const size_t capacity,
					size_t (*hash_function)(const void *key),
					int (*cmp_function)(const void *key, const void *new_key),
					void (*del_function)(void *key, void *value));

void			robin_free(t_robin *robin);

int				robin_insert(t_robin *robin, t_robin_node node);

int				robin_expand(t_robin *robin);

int				robin_remove(t_robin *robin, const void *key);

t_robin_node	*robin_search(const t_robin *robin, const void *key);

int				robin_iter(t_robin *robin,
					int (*f)(const void *key, void *value, void *args),
					void *args);
#endif

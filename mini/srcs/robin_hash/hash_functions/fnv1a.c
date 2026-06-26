/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fnv1a.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 16:30:04 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 12:33:58 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../robin_hash.h"

size_t	fnv1a(const void *key)
{
	uint64_t		hash;
	unsigned char	*str;

	hash = 14695981039346656037ULL;
	str = (unsigned char *)key;
	while (*str)
	{
		hash ^= (uint64_t)(*str++);
		hash *= 1099511628211ULL;
	}
	return ((size_t)hash);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_realloc.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 18:08:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 18:43:29 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_realloc(void *ptr, size_t elem_count, size_t elem_size)
{
	void	*new_ptr;
	size_t	old_elem_count;
	size_t	copy_count;

	if (!elem_count || !elem_size)
	{
		free(ptr);
		return (NULL);
	}
	if (elem_count <= (size_t)-1 / elem_size)
	{
		new_ptr = malloc(elem_count * elem_size);
		if (!new_ptr)
			return (NULL);
		if (!ptr)
			return (new_ptr);
		old_elem_count = ft_memlen(ptr, elem_size);
		copy_count = elem_count;
		if (old_elem_count + 1 < elem_count)
			copy_count = old_elem_count + 1;
		ft_memcpy(new_ptr, ptr, copy_count * elem_size);
		free(ptr);
		return (new_ptr);
	}
	return (NULL);
}

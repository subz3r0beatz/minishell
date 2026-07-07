/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_realloc.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 18:08:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 21:03:53 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_realloc(void *ptr, size_t elem_count, size_t elem_size)
{
	void	*new_ptr;
	size_t	old_elem_count;

	if (!elem_count || !elem_size)
	{
		free(ptr);
		new_ptr = malloc(0);
		return (new_ptr);
	}
	if (elem_count < (size_t)-1 / elem_size)
	{
		new_ptr = malloc(elem_count * elem_size);
		if (!new_ptr)
			return (NULL);
		if (!ptr)
			return (new_ptr);
		old_elem_count = ft_memlen(ptr, elem_size);
		if (old_elem_count < elem_count)
			ft_memcpy(new_ptr, ptr, old_elem_count * elem_size);
		else
			ft_memcpy(new_ptr, ptr, elem_count * elem_size);
		free(ptr);
		return (new_ptr);
	}
	return (NULL);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 15:17:43 by fldumas-          #+#    #+#             */
/*   Updated: 2025/10/25 06:40:40 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stddef.h>

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	unsigned char	*tmpdest;
	unsigned char	*tmpsrc;

	if (!dest && !src)
		return (0);
	tmpdest = (unsigned char *)dest;
	tmpsrc = (unsigned char *)src;
	if (dest <= src)
	{
		while (n)
		{
			n--;
			*tmpdest++ = *tmpsrc++;
		}
	}
	else if (dest > src)
	{
		while (n)
		{
			n--;
			*(tmpdest + n) = *(tmpsrc + n);
		}
	}
	return (dest);
}

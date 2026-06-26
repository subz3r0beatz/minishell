/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memlen.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 20:24:55 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 20:44:19 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	is_null(const char *bytes, size_t size)
{
	size_t	i;

	i = 0;
	while (i < size)
	{
		if (bytes[i] != '\0')
			return (0);
		i++;
	}
	return (1);
}

size_t	ft_memlen(const void *ptr, size_t size)
{
	size_t		len;
	const char	*bytes;

	if (!ptr || !size)
		return (0);
	bytes = (const char *)ptr;
	len = 0;
	while (1)
	{
		if (is_null(bytes, size))
			break ;
		len++;
		bytes += size;
	}
	return (len);
}

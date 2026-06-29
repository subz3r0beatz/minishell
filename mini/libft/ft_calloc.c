/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 17:21:32 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/29 18:24:54 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t nmemb, size_t size)
{
	unsigned char	*arr;
	size_t			i;

	if (!nmemb || !size)
	{
		arr = (unsigned char *)malloc(0);
		return (arr);
	}
	if (nmemb > (size_t)-1 / size)
		return (NULL);
	arr = (unsigned char *)malloc(nmemb * size);
	if (!arr)
		return (NULL);
	i = 0;
	ft_bzero(arr, nmemb * size);
	return (arr);
}

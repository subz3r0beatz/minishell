/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 17:21:32 by fldumas-          #+#    #+#             */
/*   Updated: 2025/10/25 06:39:36 by fldumas-         ###   ########.fr       */
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
	if ((nmemb * size) > (size_t)-1)
		return (0);
	arr = (unsigned char *)malloc(nmemb * size);
	if (!arr)
		return (NULL);
	i = 0;
	ft_bzero(arr, nmemb * size);
	return (arr);
}

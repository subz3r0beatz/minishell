/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 16:04:55 by fldumas-          #+#    #+#             */
/*   Updated: 2025/10/25 23:03:06 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	size_t	i;
	size_t	slen;
	char	*ns;

	if (!s)
		return (NULL);
	if (start >= ft_strlen(s))
	{
		ns = (char *)ft_calloc(sizeof(char), 1);
		return (ns);
	}
	slen = ft_strlen(s) - start;
	if (slen > len)
		slen = len;
	ns = (char *)malloc(sizeof(char) * (slen + 1));
	if (!ns)
		return (NULL);
	i = 0;
	while (i < len && s[i + start])
	{
		ns[i] = s[i + start];
		i++;
	}
	ns[i] = '\0';
	return (ns);
}

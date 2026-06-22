/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 22:26:24 by fldumas-          #+#    #+#             */
/*   Updated: 2025/10/23 01:47:31 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strtrim(char const *s1, char const *set)
{
	unsigned int	start;
	size_t			end;
	size_t			slen;
	char			*trimed;

	if (!s1 || !set)
		return (NULL);
	slen = ft_strlen(s1);
	end = slen - 1;
	while (ft_strchr(set, s1[end]) != 0 && end)
		end--;
	start = 0;
	while (ft_strchr(set, s1[start]) != 0 && start < slen - 1)
		start++;
	if (start > end)
		trimed = ft_substr(s1, 0, 0);
	else
		trimed = ft_substr(s1, start, end - start + 1);
	return (trimed);
}

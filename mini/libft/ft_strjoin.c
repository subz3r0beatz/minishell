/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 19:45:26 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/21 18:29:38 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	size_t	ls1;
	size_t	ls2;
	size_t	lns;
	char	*ns;

	if (!s1 || !s2)
	{
		ns = NULL;
		if (!s1)
			ns = ft_strdup(s2);
		else if (!s2)
			ns = ft_strdup(s1);
		if (!ns)
			return (NULL);
	}
	ls1 = ft_strlen(s1);
	ls2 = ft_strlen(s2);
	lns = ls1 + ls2;
	ns = malloc(lns + 1);
	if (!ns)
		return (NULL);
	ft_memcpy(ns, s1, ls1);
	ft_memcpy((char *)(ns + ls1), s2, ls2 + 1);
	ns[lns] = '\0';
	return (ns);
}

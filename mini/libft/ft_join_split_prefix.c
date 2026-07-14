/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_join_split_prefix.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 18:34:11 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 18:41:17 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_join_split_prefix(const char **s, const char *del)
{
	char	*str;
	size_t	i;
	size_t	elements;
	size_t	len;
	size_t	del_len;

	elements = ft_memlen(s, sizeof(char *));
	del_len = ft_strlen(del);
	i = 0;
	len = elements * del_len;
	while (s[i])
		len += ft_strlen(s[i++]);
	str = malloc((len + 1) * sizeof(char));
	if (!str)
		return (NULL);
	i = 0;
	str[0] = '\0';
	while (s[i])
	{
		ft_strlcat(str, del, len + 1);
		ft_strlcat(str, s[i], len + 1);
		i++;
	}
	return (str);
}

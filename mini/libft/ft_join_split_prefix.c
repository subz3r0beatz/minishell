/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_join_split_prefix.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 18:34:11 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 17:38:57 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	get_len(const char **s, const char *del)
{
	size_t	i;
	size_t	elements;
	size_t	len;
	size_t	del_len;

	elements = ft_memlen(s, sizeof(char *));
	if (elements == 0)
		elements = 1;
	del_len = ft_strlen(del);
	i = 0;
	len = elements * del_len;
	while (s[i])
		len += ft_strlen(s[i++]);
	return (len);
}

char	*ft_join_split_prefix(const char **s, const char *del)
{
	char	*str;
	size_t	i;
	size_t	len;

	len = get_len(s, del);
	str = malloc((len + 1) * sizeof(char));
	if (!str)
		return (NULL);
	i = 0;
	str[0] = '\0';
	if (!s[0])
		ft_strlcat(str, del, len + 1);
	while (s[i])
	{
		ft_strlcat(str, del, len + 1);
		ft_strlcat(str, s[i], len + 1);
		i++;
	}
	return (str);
}

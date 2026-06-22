/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 11:57:32 by fldumas-          #+#    #+#             */
/*   Updated: 2025/10/25 06:13:02 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	ft_getwords_len(char const *s, char c, int len)
{
	size_t	i;
	size_t	words;

	if (len)
	{
		i = 1;
		words = 1;
		while (s[i])
		{
			if (s[i] == c && s[i - 1] != c && s[i])
				words++;
			i++;
		}
		if (s[i - 1] == c)
			words--;
		return (words);
	}
	else
	{
		i = 0;
		while (s[i] && s[i] != c)
			i++;
		return (i);
	}
	return (0);
}

static void	ft_freeall(char **strs, size_t	i)
{
	while (i)
	{
		i--;
		free(strs[i]);
	}
	free(strs);
	return ;
}

static char	**ft_getsplits(char **strs, const char *s, char c, size_t words)
{
	size_t	i;
	size_t	j;
	size_t	wordlen;

	i = 0;
	j = 0;
	while (i < words)
	{
		while (!ft_getwords_len(s + j, c, 0))
			j++;
		wordlen = ft_getwords_len(s + j, c, 0);
		strs[i] = ft_substr(s, j, wordlen);
		if (!strs[i])
		{
			ft_freeall(strs, i);
			return (NULL);
		}
		j += wordlen;
		i++;
	}
	strs[i] = NULL;
	return (strs);
}

char	**ft_split(char const *s, char c)
{
	char	**strs;
	size_t	words;

	if (!s || !*s)
	{
		strs = (char **)ft_calloc(sizeof(char *), 1);
		return (strs);
	}
	words = ft_getwords_len(s, c, 1);
	strs = (char **)malloc(sizeof(char *) * (words + 1));
	if (!strs)
		return (NULL);
	strs = ft_getsplits(strs, s, c, words);
	return (strs);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_char_split_quotes_exl.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 14:20:52 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 15:56:50 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	count_words(char *str, int (*delim)(int))
{
	size_t	i;
	size_t	count;
	char	quote_state;

	i = 0;
	count = 0;
	quote_state = 0;
	while (str[i])
	{
		while (str[i] && delim(str[i]))
			i++;
		if (!str[i])
			break ;
		count++;
		while (str[i] && (quote_state || !delim(str[i])))
		{
			if (quote_state == 0 && (str[i] == '\'' || str[i] == '"'))
				quote_state = str[i];
			else if (quote_state && str[i] == quote_state)
				quote_state = 0;
			i++;
		}
	}
	return (count);
}

static size_t	get_world_len(char *str, int (*delim)(int),
	size_t *stripped_len)
{
	size_t	len;
	char	quote_state;

	len = 0;
	*stripped_len = 0;
	quote_state = 0;
	while (str[len] && (quote_state || !delim(str[len])))
	{
		if (quote_state == 0 && (str[len] == '\'' || str[len] == '"'))
			quote_state = str[len];
		else if (quote_state && str[len] == quote_state)
			quote_state = 0;
		else
			(*stripped_len)++;
		len++;
	}
	return (len);
}

static void	copy_word(char *dst, char *src, size_t word_len)
{
	size_t	i;
	size_t	j;
	char	quote_state;

	i = 0;
	j = 0;
	quote_state = 0;
	while (i < word_len)
	{
		if (quote_state == 0 && (src[i] == '\'' || src[i] == '"'))
			quote_state = src[i];
		else if (quote_state && src[i] == quote_state)
			quote_state = 0;
		else
			dst[j++] = src[i];
		i++;
	}
	dst[j] = '\0';
}

static char	**get_splits(char **split, char *str, int (*delim)(int))
{
	size_t	i;
	size_t	count;
	size_t	world_len;
	size_t	stripped_len;

	i = 0;
	count = 0;
	while (str[i])
	{
		while (str[i] && delim(str[i]))
			i++;
		if (!str[i])
			break ;
		world_len = get_world_len(&str[i], delim, &stripped_len);
		split[count] = malloc((stripped_len + 1) * sizeof(char));
		if (!split[count])
		{
			ft_free_matrix(split, count);
			return (NULL);
		}
		copy_word(split[count], &str[i], world_len);
		count++;
		i += world_len;
	}
	return (split);
}

char	**ft_char_split_quotes_exl(char *str, int (*delim)(int))
{
	char	**split;
	size_t	word_count;

	if (!str || !delim)
		return (NULL);
	word_count = count_words(str, delim);
	split = malloc((word_count + 1) * sizeof(char *));
	if (!split)
		return (NULL);
	split = get_splits(split, str, delim);
	if (split)
		split[word_count] = NULL;
	return (split);
}

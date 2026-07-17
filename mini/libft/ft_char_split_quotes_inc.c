/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_char_split_quotes_inc.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 15:29:25 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 15:55:26 by fldumas-         ###   ########.fr       */
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

static size_t	get_world_len(char *str, int (*delim)(int))
{
	size_t	len;
	char	quote_state;

	len = 0;
	quote_state = 0;
	while (str[len] && (quote_state || !delim(str[len])))
	{
		if (quote_state == 0 && (str[len] == '\'' || str[len] == '"'))
			quote_state = str[len];
		else if (quote_state && str[len] == quote_state)
			quote_state = 0;
		len++;
	}
	return (len);
}

static char	**get_splits(char **split, char *str, int (*delim)(int))
{
	size_t	i;
	size_t	count;
	size_t	world_len;

	i = 0;
	count = 0;
	while (str[i])
	{
		while (str[i] && delim(str[i]))
			i++;
		if (!str[i])
			break ;
		world_len = get_world_len(&str[i], delim);
		split[count] = malloc((world_len + 1) * sizeof(char));
		if (!split[count])
		{
			ft_free_matrix(split, count);
			return (NULL);
		}
		ft_strlcpy(split[count], &str[i], world_len + 1);
		count++;
		i += world_len;
	}
	split[count] = NULL;
	return (split);
}

char	**ft_char_split_quotes_inc(char *str, int (*delim)(int))
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
	return (split);
}

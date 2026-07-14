/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split_quotes.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 15:29:25 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 20:48:14 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	count_words(char *str, char *delim, size_t delim_len)
{
	size_t	i;
	size_t	count;
	char	quote_state;

	i = 0;
	count = 0;
	quote_state = 0;
	while (str[i])
	{
		while (str[i] && !ft_strncmp(&str[i], delim, delim_len))
			i += delim_len;
		if (!str[i])
			break ;
		count++;
		while (str[i] && (quote_state || ft_strncmp(&str[i], delim, delim_len)))
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

static char	**get_splits(char **split, char *str, char *delim, size_t delim_len)
{
	size_t	i;
	char	quote_state;


}

char	**ft_split_quote(char *str, char *delim)
{
	char	**split;
	size_t	delim_len;

	if (!str || !delim || !delim[0]
		|| ft_strchr(delim, '"') || ft_strchr(delim, '\''))
		return (NULL);
	delim_len = ft_strlen(delim);
	split = malloc((count_words(str, delim, delim_len) + 1) * sizeof(char *));
	if (!split)
		return (NULL);
	split = get_splits(split, str, delim, delim_len);
	return (split);
}

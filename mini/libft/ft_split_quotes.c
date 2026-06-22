/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   split_quotes.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 15:29:25 by fldumas-          #+#    #+#             */
/*   Updated: 2026/04/07 00:12:26 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

static void	free_split(char **split, size_t size)
{
	size_t	i;

	i = 0;
	while (i < size)
	{
		free(split[i]);
		i++;
	}
	free(split);
}

static size_t	word_count(const char *s, char c)
{
	size_t	i;
	size_t	count;
	int		single_quote;
	int		double_quote;

	i = 0;
	count = 0;
	single_quote = 0;
	double_quote = 0;
	while (s[i])
	{
		while (s[i] && s[i] == c && !single_quote && !double_quote)
			i++;
		if (s[i] && s[i] != c)
			count++;
		while (s[i] && (s[i] != c || single_quote || double_quote))
		{
			if (s[i] == '\'')
				single_quote = !single_quote;
			else if (s[i] == '"')
				double_quote = !double_quote;
			i++;
		}
	}
	return (count);
}

static size_t	word_len(const char *s, char c)
{
	size_t	i;
	int		single_quote;
	int		double_quote;

	i = 0;
	single_quote = 0;
	double_quote = 0;
	while (s[i] && (s[i] != c || single_quote || double_quote))
	{
		if (s[i] == '\'')
			single_quote = !single_quote;
		else if (s[i] == '"')
			double_quote = !double_quote;
		i++;
	}
	return (i);
}

static void	trim_quotes(char **split)
{
	size_t	i;
	char	*trimed;

	i = 0;
	while (split[i])
	{
		trimed = ft_strtrim(split[i], "\"'");
		if (!trimed)
		{
			write(2, "Error: memory allocation failed\n", 32);
			free_matrix(split);
			exit(1);
		}
		free(split[i]);
		split[i] = trimed;
		i++;
	}
}

char	**split_quotes(const char *s, char c)
{
	char	**split;
	size_t	i[2];

	split = ft_calloc(sizeof(char *), word_count(s, c) + 1);
	if (!split)
		return (NULL);
	ft_bzero(i, sizeof(size_t) * 2);
	while (s[i[0]])
	{
		while (s[i[0]] && s[i[0]] == c)
			i[0]++;
		if (s[i[0]])
		{
			split[i[1]] = ft_substr(s, i[0], word_len(s + i[0], c));
			if (!split[i[1]])
			{
				free_split(split, i[1]);
				return (NULL);
			}
			i[1]++;
			i[0] += word_len(s + i[0], c);
		}
	}
	trim_quotes(split);
	return (split);
}

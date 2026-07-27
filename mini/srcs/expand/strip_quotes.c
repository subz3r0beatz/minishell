/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strip_quotes.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 21:39:20 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/27 23:49:15 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	get_stripped_len(char *word)
{
	size_t	i;
	size_t	len;
	char	quote_state;

	i = 0;
	len = 0;
	quote_state = 0;
	while (word[i])
	{
		if (quote_state == 0 && (word[i] == '"' || word[i] == '\''))
			quote_state = word[i];
		else if (quote_state && word[i] == quote_state)
			quote_state = 0;
		else
			len++;
		i++;
	}
	return (len);
}

char	*strip_quotes(char *word)
{
	char	*new_word;
	char	quote_state;
	size_t	i;
	size_t	j;

	new_word = malloc(sizeof(char) * (get_stripped_len(word) + 1));
	if (new_word)
	{
		i = 0;
		j = 0;
		quote_state = 0;
		while (word[i])
		{
			if (quote_state == 0 && (word[i] == '"' || word[i] == '\''))
				quote_state = word[i];
			else if (quote_state && word[i] == quote_state)
				quote_state = 0;
			else
				new_word[j++] = word[i];
			i++;
		}
		new_word[j] = '\0';
	}
	free(word);
	return (new_word);
}

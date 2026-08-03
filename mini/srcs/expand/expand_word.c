/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_word.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 23:22:23 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/03 03:59:01 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*copy_word(char *word, char *value, size_t key_len, size_t *i)
{
	size_t	word_len;
	size_t	value_len;
	char	*new_word;

	value_len = 0;
	if (value)
		value_len = ft_strlen(value);
	word_len = *i + value_len + ft_strlen(word + *i + 1 + key_len);
	new_word = malloc(sizeof(char) * (word_len + 1));
	if (new_word)
	{
		ft_strlcpy(new_word, word, *i + 1);
		if (value)
			ft_strlcat(new_word, value, word_len + 1);
		ft_strlcat(new_word, word + *i + 1 + key_len, word_len + 1);
		*i += value_len - 1;
	}
	free(word);
	return (new_word);
}

static char	*expand_variable(t_minishell *shell, char *word, size_t *i)
{
	char	*new_word;
	char	*key;
	char	*value;
	size_t	key_len;

	key_len = 0;
	while (word[*i + 1 + key_len] && (ft_isalnum(word[*i + 1 + key_len])
			|| word[*i + 1 + key_len] == '_'))
		key_len++;
	if (!key_len)
		return (word);
	key = ft_substr(word, *i + 1, key_len);
	if (!key)
	{
		free(word);
		return (NULL);
	}
	get_var_value(shell, key, &value);
	new_word = copy_word(word, value, key_len, i);
	free(key);
	return (new_word);
}



char	*expand_word(t_minishell *shell, char *word)
{
	char	*new_word;
	size_t	word_len;

	word_len = calculate_word_len(shell, word);
	new_word = copy_new_word(word);
	i = 0;
	quote_state = 0;
	while (word[i])
	{
		if (quote_state == 0 && (word[i] == '"' || word[i] == '\''))
			quote_state = word[i];
		else if (quote_state && word[i] == quote_state)
			quote_state = 0;
		if (word[i] == '$' && quote_state != '\'')
		{
			if (word[i + 1] == '?' || word[i + 1] == '0'
				|| word[i + 1] == '$' || word[i + 1] == '!')
				word = expand_special_param(shell, word, &i);
			else
				word = expand_variable(shell, word, &i);
		}
		if (!word)
			return (NULL);
		i++;
	}
	return (word);
}

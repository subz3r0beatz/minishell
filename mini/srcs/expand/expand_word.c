/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_word.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 23:22:23 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 03:41:45 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	count_var_len(t_minishell *shell, char *word, size_t *i)
{
	size_t	key_len;
	char	*value;
	char	saved;

	if (word[0] == '?' || word[0] == '!' || word[0] == '0' || word[0] == '$')
		return (count_special_param_len(shell, word[0], i));
	if (!ft_isalpha(word[0]) && word[0] != '_')
	{
		if (ft_isdigit(word[0]))
		{
			++*i;
			return (0);
		}
		return (1);
	}
	key_len = 1;
	while (word[key_len] && (ft_isalnum(word[key_len]) || word[key_len] == '_'))
		key_len++;
	saved = word[key_len];
	word[key_len] = 0;
	get_var_value(shell, word, &value);
	word[key_len] = saved;
	*i += key_len;
	return (ft_strlen(value));
}

static size_t	calculate_word_len(t_minishell *shell, char *word)
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
		else if (word[i] == '$' && quote_state != '\'')
		{
			if (quote_state == '"'
				|| (word[i + 1] != '"' && word[i + 1] != '\''))
				len += count_var_len(shell, &word[i + 1], &i);
		}
		else
			len++;
		i++;
	}
	return (len);
}

static size_t	copy_var_value(t_minishell *shell, char *new_word, char *word,
	size_t *i)
{
	size_t	key_len;
	char	*value;
	char	saved;

	if (word[0] == '?' || word[0] == '!' || word[0] == '0' || word[0] == '$')
		return (copy_special_param(shell, new_word, word[0], i));
	if (!ft_isalpha(word[0]) && word[0] != '_')
	{
		if (ft_isdigit(word[0]))
			++*i;
		if (ft_isdigit(word[0]))
			return (0);
		new_word[0] = '$';
		return (1);
	}
	key_len = 1;
	while (word[key_len] && (ft_isalnum(word[key_len]) || word[key_len] == '_'))
		key_len++;
	saved = word[key_len];
	word[key_len] = 0;
	get_var_value(shell, word, &value);
	word[key_len] = saved;
	*i += key_len;
	ft_memcpy(new_word, value, ft_strlen(value));
	return (ft_strlen(value));
}

static void	copy_new_word(t_minishell *shell, char *new_word, char *word)
{
	size_t	i;
	size_t	j;
	char	quote_state;

	i = 0;
	j = 0;
	quote_state = 0;
	while (word[i])
	{
		if (quote_state == 0 && (word[i] == '"' || word[i] == '\''))
			quote_state = word[i];
		else if (quote_state && quote_state == word[i])
			quote_state = 0;
		else if (word[i] == '$' && quote_state != '\'')
		{
			if (quote_state == '"'
				|| (word[i + 1] != '"' && word[i + 1] != '\''))
				j += copy_var_value(shell, &new_word[j], &word[i + 1], &i);
		}
		else
			new_word[j++] = word[i];
		i++;
	}
	new_word[j] = 0;
}

char	*expand_word(t_minishell *shell, char *word)
{
	char	*new_word;
	size_t	word_len;

	if (!word)
		return (NULL);
	word_len = calculate_word_len(shell, word);
	new_word = malloc((word_len + 1) * sizeof(char));
	if (!new_word)
	{
		free(word);
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	copy_new_word(shell, new_word, word);
	free(word);
	return (new_word);
}

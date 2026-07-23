/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 15:58:21 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/17 02:08:29 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*expand_variable(t_minishell *shell, char *word, size_t *i)
{
	char	*new_word;
	char	*value;
	size_t	key_len;
	size_t	value_len;
	size_t	word_len;

	key_len = 0;
	while (word[*i + 1 + key_len] && (ft_isalnum(word[*i + 1 + key_len])
			|| word[*i + 1 + key_len] == '_'))
		key_len++;
	if (get_var_value(shell, word + *i + 1, &value) || !value)
		value_len = 0;
	else
		value_len = ft_strlen(value);
	word_len = *i + value_len + ft_strlen(word + *i + 1 + key_len);
	new_word = malloc(sizeof(char) * (word_len + 1));
	if (new_word)
	{
		ft_strlcpy(new_word, word, *i);
		ft_strlcat(new_word, value, word_len + 1);
		ft_strlcat(new_word, word + *i + 1 + key_len, word_len + 1);
		*i += value_len - 1;
	}
	free(word);
	return (new_word);
}

static int	is_param(char c)
{
	if (c == '?' || c == '_' || isdigit(c) || c == '#'
		|| c == '*' || c == '!' || c == '_' || c == '@')
		return (1);
	return (0);
}

static char	*expand_word(t_minishell *shell, char *word)
{
	size_t	i;
	char	quote_state;

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
			if (is_param(word[i + 1]))
				word = handle_special_param(shell, word, &i);
			else
				word = expand_variable(shell, word, &i);
		}
		if (!word)
			return (NULL);
		i++;
	}
	return (word);
}

int	expand(t_minishell *shell, t_ast_node *node)
{
	size_t	i;

	i = 0;
	while (node->args && node->args[i])
	{
		if (node->args[i][0] == '~' && !node->args[i][1])
			node->args[i] = expand_home(shell, node->args[i]);
		else
			node->args[i] = expand_word(shell, node->args[i]);
		if (!node->args[i])
			return (1);
		i++;
	}
	return (0);
}

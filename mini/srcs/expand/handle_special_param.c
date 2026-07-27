/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_special_param.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 23:50:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/26 16:32:14 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*get_param_value(t_minishell *shell, char *argv0, char param)
{
	if (param == '?')
		return (ft_itoa(shell->exit_status));
	if (param == '0')
		return (ft_strdup(argv0));
	if (param == '$' && shell->pid)
		return (ft_strdup(shell->pid));
	if (param == '!' && shell->last_pid)
		return (ft_strdup(shell->last_pid));
	return (ft_strdup(""));
}

char	*handle_special_param(t_minishell *shell, char *argv0,
	char *word, size_t *i)
{
	char	*value;
	char	*new_word;
	size_t	value_len;
	size_t	word_len;

	value = get_param_value(shell, argv0, word[*i + 1]);
	if (!value)
	{
		free(word);
		return (NULL);
	}
	value_len = ft_strlen(value);
	word_len = *i + value_len + ft_strlen(word + *i + 2);
	new_word = malloc(sizeof(char) * (word_len + 1));
	if (new_word)
	{
		ft_strlcpy(new_word, word, *i + 1);
		ft_strlcat(new_word, value, word_len + 1);
		ft_strlcat(new_word, word + *i + 2, word_len + 1);
		*i += value_len - 1;
	}
	free(word);
	free(value);
	return (new_word);
}

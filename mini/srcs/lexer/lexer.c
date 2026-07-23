/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 09:22:01 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 17:03:18 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	get_word_len(char *input, uint8_t table[256][256])
{
	size_t	len;
	char	quote_state;

	len = 0;
	quote_state = 0;
	while (input[len])
	{
		if (quote_state == 0 && (input[len] == '"' || input[len] == '\''))
			quote_state = input[len];
		else if (quote_state == input[len])
			quote_state = 0;
		if (quote_state == 0 && (ft_iswhite(input[len])
				|| (t_token_type)table[(unsigned char)input[len]]
				[(unsigned char)input[len + 1]] != TOKEN_WORD))
			break ;
		len++;
	}
	return (len);
}

static t_token	*handle_token(char *input, size_t *i,
		uint8_t table[256][256])
{
	size_t	len;
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type
		= (t_token_type)table[(unsigned char)input[0]][(unsigned char)input[1]];
	len = 0;
	if (token->type == TOKEN_WORD)
		len = get_word_len(input, table);
	else if (token->type == TOKEN_OR || token->type == TOKEN_AND
		|| token->type == TOKEN_DLESS || token->type == TOKEN_DGREAT)
		len = 2;
	else
		len = 1;
	token->value = ft_substr(input, 0, len);
	if (!token->value)
	{
		free(token);
		return (NULL);
	}
	token->next = NULL;
	*i += len;
	return (token);
}

static void	free_tokens(t_token *head)
{
	t_token	*tmp;

	while (head)
	{
		tmp = head;
		head = head->next;
		free(tmp->value);
		free(tmp);
	}
}

static t_token	*add_token(t_token *head, t_token *token)
{
	t_token	*tmp;

	if (!head)
		return (token);
	tmp = head;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = token;
	return (head);
}

t_token	*lexer(char *input, uint8_t table[256][256])
{
	t_token			*head;
	t_token			*token;
	size_t			i;

	if (!input)
		return (NULL);
	head = NULL;
	i = 0;
	while (input[i])
	{
		while (input[i] && ft_iswhite(input[i]))
			i++;
		if (!input[i])
			break ;
		token = handle_token(&input[i], &i, table);
		if (!token)
		{
			free_tokens(head);
			return (NULL);
		}
		head = add_token(head, token);
	}
	return (head);
}

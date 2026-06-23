/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 09:22:01 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/23 17:14:52 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	get_word_len(char *input, t_token_type table[256][256])
{
	size_t	len;
	int		s_quote;
	int		d_quote;

	len = 0;
	s_quote = 0;
	d_quote = 0;
	while (input[len])
	{
		if (input[len] == '\"' && s_quote == 0)
			d_quote = !d_quote;
		else if (input[len] == '\'' && d_quote == 0)
			s_quote = !s_quote;
		if (!s_quote && !d_quote && (ft_iswhite(input[len])
				|| table[(unsigned char)input[len]]
				[(unsigned char)input[len + 1]] != TOKEN_WORD))
			break ;
		len++;
	}
	return (len);
}

static t_token	*handle_token(char *input, size_t *i,
		t_token_type table[256][256])
{
	size_t	len;
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = table[(unsigned char)input[0]][(unsigned char)input[1]];
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

t_token	*lexer(char *input)
{
	t_token			*head;
	t_token			*token;
	t_token_type	table[256][256];
	size_t			i;

	head = NULL;
	i = 0;
	init_token_type_table(table);
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

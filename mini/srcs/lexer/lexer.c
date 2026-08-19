/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 09:22:01 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/18 19:55:55 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	handle_token_type(char *input, t_token *token,
	uint8_t table[256][256])
{
	if (input[0] == '<' && input[1] == '<' && input[2] == '<')
	{
		token->type = TOKEN_TLESS;
		return (3);
	}
	else
	{
		token->type
			= (t_token_type)table
		[(unsigned char)input[0]][(unsigned char)input[1]];
		if (token->type == TOKEN_WORD || token->type == TOKEN_COMMENT)
			return (get_word_token_len(input, table));
		else if (token->type == TOKEN_OR || token->type == TOKEN_AND
			|| token->type == TOKEN_DLESS || token->type == TOKEN_DGREAT)
			return (2);
		else if (input[0] == ';' && input[1] == '&')
			return (2);
		else if (input[0] == ';' && input[1] == ';' && input[2] == '&')
			return (3);
		else if (input[0] == ';' && input[1] == ';' && input[2] != '&')
			return (2);
		return (1);
	}
}

static t_token	*handle_token(char *input, size_t *i,
		uint8_t table[256][256])
{
	size_t	len;
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	len = handle_token_type(input, token, table);
	token->value = ft_substr(input, 0, len);
	if (!token->value)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		free(token);
		return (NULL);
	}
	token->next = NULL;
	*i += len;
	return (token);
}

static int	skip_token(char *input, size_t *i, uint8_t table[256][256])
{
	while (input[*i] && ft_iswhite(input[*i]) && input[*i] != '\n')
		++*i;
	if (!input[*i])
		return (1);
	if ((t_token_type)table[(unsigned char)input[*i]]
		[(unsigned char)input[*i + 1]] == TOKEN_COMMENT)
	{
		while (input[*i] && input[*i] != '\n')
			++*i;
		return (1);
	}
	return (0);
}

t_token	*lexer(char *input, uint8_t table[256][256])
{
	t_token			*head;
	t_token			*tail;
	t_token			*token;
	size_t			i;

	head = NULL;
	tail = NULL;
	i = 0;
	while (input[i])
	{
		if (skip_token(input, &i, table))
			continue ;
		token = handle_token(&input[i], &i, table);
		if (!token)
			return (free_tokens(head));
		if (!head)
			head = token;
		else
			tail->next = token;
		tail = token;
	}
	return (head);
}

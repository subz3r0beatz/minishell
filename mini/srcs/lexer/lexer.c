/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 09:22:01 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/22 16:43:37 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_operator(char *input)
{
	if (ft_strncmp(input, "||", 2) == 0)
		return (1);
	if (ft_strncmp(input, "&&", 2) == 0)
		return (1);
	if (ft_strncmp(input, "<<", 2) == 0)
		return (1);
	if (ft_strncmp(input, ">>", 2) == 0)
		return (1);
	if (input[0] == '|')
		return (1);
	if (input[0] == '&')
		return (1);
	if (input[0] == '<')
		return (1);
	if (input[0] == '>')
		return (1);
	return (0);
}

static t_token	*handle_operator(char *input)
{
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = TOKEN_WORD;
	token->value = ft_strdup(input);
	token->next = NULL;
	return (token);
}

static t_token	*tokenize(char *input)
{
	t_token	*head;
	t_token	*token;
	size_t	i;

	head = NULL;
	i = 0;
	while (input[i])
	{
		while (input[i] && ft_iswhite(input[i]))
			i++;
		if (!input[i])
			break ;
		if (is_operator(&input[i]))
			token = handle_operator(&input[i]);
		else
			token = handle_word(&input[i]);
		if (!token)
			return (NULL);
		add_token(&head, token);
	}
	return (tokens);
}

t_token	*lexer(char *input)
{
	t_token	*tokens;

	tokens = tokenize(input);
	return (tokens);
}

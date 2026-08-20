/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_unclosed_quotes.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 14:38:51 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/19 04:32:45 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*sigint_sigquit(t_minishell *shell, char *next,
	char quote, int *malloc_error)
{
	if (!malloc_error || (!next && *malloc_error))
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		if (next)
			free(next);
		return (NULL);
	}
	shell->syn_err = 1;
	if (g_signal_status == 130)
	{
		shell->exit_status = 130;
		free(next);
		return (NULL);
	}
	ft_putstr_fd("minishell: unexpected EOF while "
		"looking for matching `", STDERR_FILENO);
	ft_putchar_fd(quote, STDERR_FILENO);
	ft_putstr_fd("'\nminishell: syntax error: "
		"unexpected end of file\n", STDERR_FILENO);
	shell->exit_status = 2;
	return (NULL);
}

static char	*get_next(t_minishell *shell, char *buffer, char quote)
{
	int		malloc_error;
	char	*next;
	char	*tmp;

	malloc_error = 0;
	rl_event_hook = rl_signal_check;
	if (isatty(STDIN_FILENO))
		next = readline("> ");
	else
		next = ft_gnl(STDIN_FILENO, buffer, 128, &malloc_error);
	rl_event_hook = NULL;
	if (!next || g_signal_status == 130)
		return (sigint_sigquit(shell, next, quote, &malloc_error));
	if (!shell->history)
		return (next);
	if (isatty(STDIN_FILENO))
		tmp = ft_strjoin_3(shell->history, "\n", next);
	if (!tmp)
		return (sigint_sigquit(shell, next, 0, NULL));
	free(shell->history);
	shell->history = tmp;
	return (next);
}

static int	read_open_quotes(t_minishell *shell, t_token *token, char *buffer)
{
	char	*next;
	char	*tmp;
	char	quote;

	buffer[0] = '\0';
	while (check_unclosed_quotes(token->value, &quote))
	{
		g_signal_status = 0;
		next = get_next(shell, buffer, quote);
		if (!next)
			return (1);
		if (isatty(STDIN_FILENO))
			tmp = ft_strjoin_3(token->value, "\n", next);
		else
			tmp = ft_strjoin(token->value, next);
		free(next);
		if (!tmp)
			sigint_sigquit(shell, NULL, 0, NULL);
		if (!tmp)
			return (1);
		free(token->value);
		token->value = tmp;
	}
	return (0);
}

static int	lex_new_token(t_minishell *shell, t_token *token,
	size_t len, char *rest)
{
	char	*tmp;
	t_token	*new_token;
	t_token	*tail;

	tmp = ft_substr(token->value, 0, len);
	if (!tmp)
	{
		free(rest);
		return (1);
	}
	free(token->value);
	token->value = tmp;
	new_token = lexer(rest, shell->token_type_table);
	free(rest);
	if (new_token)
	{
		tail = new_token;
		while (tail->next)
			tail = tail->next;
		tail->next = token->next;
		token->next = new_token;
	}
	return (0);
}

int	parse_unclosed_quotes(t_minishell *shell, t_token *token)
{
	size_t	len;
	char	*rest;
	char	buffer[128];

	if (read_open_quotes(shell, token, buffer))
		return (1);
	len = get_word_token_len(token->value, shell->token_type_table);
	if (token->value[len])
	{
		rest = NULL;
		rest = ft_strdup(token->value + len);
		if (!rest || lex_new_token(shell, token, len, rest))
		{
			ft_putstr_fd("minishell: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			return (1);
		}
	}
	return (0);
}

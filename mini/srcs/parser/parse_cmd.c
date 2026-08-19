/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_cmd.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 15:32:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/19 04:21:32 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	collect_heredocs(t_minishell *shell, t_redir *redir)
{
	int	sig;

	while (redir)
	{
		if (redir->type == TOKEN_DLESS)
		{
			sig = 0;
			redir->fd = handle_heredoc(shell, redir->file, &sig);
			if (sig || g_signal_status == 130)
			{
				shell->exit_status = 130;
				shell->syn_err = 1;
				return (1);
			}
			if (redir->fd < 0)
			{
				shell->syn_err = 1;
				return (1);
			}
		}
		redir = redir->next;
	}
	return (0);
}

static char	**add_to_matrix(char **matrix, char *str)
{
	char	**new_matrix;
	size_t	len;

	len = ft_memlen(matrix, sizeof(char *));
	new_matrix = ft_realloc(matrix, len + 2, sizeof(char *));
	if (!new_matrix)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		ft_free_matrix(matrix, len);
		return (NULL);
	}
	new_matrix[len] = ft_strdup(str);
	if (!new_matrix[len])
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		ft_free_matrix(new_matrix, len);
		return (NULL);
	}
	new_matrix[len + 1] = NULL;
	return (new_matrix);
}

static t_ast_node	*parse_subshell(t_minishell *shell, t_token **token)
{
	t_ast_node	*sub_ast;
	t_ast_node	*node;

	*token = (*token)->next;
	sub_ast = parse_list(shell, token);
	if (!sub_ast)
		return (NULL);
	if (!*token || (*token)->type != TOKEN_RPAREN)
		syntax_error(shell, *token);
	if (!*token || (*token)->type != TOKEN_RPAREN)
		return (free_ast(sub_ast));
	*token = (*token)->next;
	node = new_op_node(NODE_SUBSHELL, sub_ast, NULL);
	if (!node)
		return (free_ast(sub_ast));
	while (*token && ((*token)->type == TOKEN_LESS
			|| (*token)->type == TOKEN_GREAT || (*token)->type == TOKEN_DLESS
			|| (*token)->type == TOKEN_DGREAT || (*token)->type == TOKEN_TLESS))
	{
		if (parse_redir(shell, token, &node->redir))
			return (free_ast(node));
	}
	if (collect_heredocs(shell, node->redir))
		return (free_ast(node));
	return (node);
}

static t_ast_node	*loop_tokens(t_minishell *shell, t_token **token,
	t_ast_node *node)
{
	while (*token && ((*token)->type == TOKEN_WORD
			|| (*token)->type == TOKEN_LESS || (*token)->type == TOKEN_GREAT
			|| (*token)->type == TOKEN_DLESS || (*token)->type == TOKEN_DGREAT
			|| (*token)->type == TOKEN_TLESS))
	{
		if ((*token)->type == TOKEN_WORD)
		{
			if (check_unclosed_quotes((*token)->value, NULL))
				if (parse_unclosed_quotes(shell, *token))
					return (free_ast(node));
			node->args = add_to_matrix(node->args, (*token)->value);
			if (!node->args)
				return (free_ast(node));
			*token = (*token)->next;
		}
		else if (parse_redir(shell, token, &node->redir))
			return (free_ast(node));
	}
	if (!node->args && !node->redir)
		return (free_ast(node));
	if (*token && (*token)->type == TOKEN_LPAREN)
		free_ast(node);
	if (*token && (*token)->type == TOKEN_LPAREN)
		return (syntax_error(shell, *token));
	return (node);
}

t_ast_node	*parse_cmd(t_minishell *shell, t_token **token)
{
	t_ast_node	*node;

	if (!token || !*token)
		return (NULL);
	if ((*token)->type == TOKEN_LPAREN)
		return (parse_subshell(shell, token));
	if ((*token)->type != TOKEN_WORD && (*token)->type != TOKEN_LESS
		&& (*token)->type != TOKEN_GREAT && (*token)->type != TOKEN_DLESS
		&& (*token)->type != TOKEN_DGREAT && (*token)->type != TOKEN_TLESS)
		return (syntax_error(shell, *token));
	if (new_cmd_node(&node))
		return (NULL);
	node = loop_tokens(shell, token, node);
	if (!node)
		return (NULL);
	if (collect_heredocs(shell, node->redir))
		return (free_ast(node));
	return (node);
}

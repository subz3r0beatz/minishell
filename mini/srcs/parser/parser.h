/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 17:47:49 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 02:29:06 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include "ast/ast.h"

typedef struct s_minishell	t_minishell;
typedef enum e_token_type	t_token_type;

typedef enum e_node_type
{
	NODE_CMD,
	NODE_PIPE,
	NODE_AND,
	NODE_OR,
	NODE_SUBSHELL,
	NODE_SEMI,
	NODE_BACKGR
}			t_node_type;

typedef struct s_redir
{
	t_token_type	type;
	int				fd;
	char			*file;
	struct s_redir	*next;
}				t_redir;

typedef struct s_ast_node
{
	t_node_type			type;
	struct s_ast_node	*left;
	struct s_ast_node	*right;
	char				**args;
	t_redir				*redir;
}				t_ast_node;

int			parse_redir(t_minishell *shell, t_token **token,
				t_redir **redir_head);
t_ast_node	*parse_cmd(t_minishell *shell, t_token **token);
t_ast_node	*parse_pipeline(t_minishell *shell, t_token **token);
t_ast_node	*parse_logic(t_minishell *shell, t_token **token);
t_ast_node	*parse_list(t_minishell *shell, t_token **token);
t_ast_node	*syntax_error(t_minishell *shell, t_token *token);

t_ast_node	*parser(t_minishell *shell, t_token *tokens);

#endif

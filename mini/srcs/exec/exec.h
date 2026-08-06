/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 14:36:16 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 03:43:23 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXEC_H
# define EXEC_H

int		restore_fds(int in, int out);
int		init_saved_std(t_minishell *shell, t_redir *redir,
				int *in, int *out);
int		exec(t_minishell *shell, t_ast_node *node);
int		exec_and(t_minishell *shell, t_ast_node *node);
int		exec_or(t_minishell *shell, t_ast_node *node);
int		exec_pipe(t_minishell *shell, t_ast_node *node);
int		exec_cmd(t_minishell *shell, t_ast_node *node);
int		exec_binary(t_minishell *shell, t_ast_node *node);
int		exec_pipe(t_minishell *shell, t_ast_node *node);
int		exec_subshell(t_minishell *shell, t_ast_node *node);
int		exec_semi(t_minishell *shell, t_ast_node *node);
int		exec_backgr(t_minishell *shell, t_ast_node *node);
int		apply_redirections(t_minishell *shell, t_redir *redir);
int		handle_heredoc(t_minishell *shell, char *file);
int		is_builtin(char *cmd);
int		exec_builtin(t_minishell *shell, t_ast_node *node, int builtin);
char	*clean_quotes(char *word);
char	*get_path(t_minishell *shell, char *cmd, int *exists);

#endif

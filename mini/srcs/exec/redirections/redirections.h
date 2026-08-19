/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 18:53:15 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/18 16:31:24 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REDIRECTIONS_H
# define REDIRECTIONS_H

int	redirections(t_redir *redir);
int	restore_fds(int in, int out);
int	handle_herestring(char *file);
int	handle_heredoc(t_minishell *shell, char *file, int *sigint_status);
//int	collect_heredocs(t_minishell *shell, t_ast_node *node);
int	heredoc_read_loop(t_minishell *shell, int pfd[2],
		char *limiter, int expand);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 19:52:59 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 14:43:54 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXPAND_H
# define EXPAND_H

int		expand(t_minishell *shell, t_ast_node *node);
int		expand_redirs(t_minishell *shell, t_redir *redir);
size_t	count_special_param_len(t_minishell *shell, char param, size_t *i);
size_t	copy_special_param(t_minishell *shell, char *new_word, char param,
			size_t *i);
char	*expand_home(t_minishell *shell, char *word);
char	*expand_word(t_minishell *shell, char *word);
char	*get_home(void);

#endif

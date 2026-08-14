/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prompt.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 06:21:12 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/03 03:18:09 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROMPT_H
# define PROMPT_H

typedef struct s_minishell	t_minishell;

void		build_prompt(t_minishell *shell);
char	*get_username(t_minishell *shell, int *malloc_error);
char	*get_hostname(t_minishell *shell, int *malloc_error);
char	*get_prompt_pwd(t_minishell *shell, int *malloc_error);
ssize_t	open_read(char *path, char *buffer, size_t size);
char	*search_passwd(size_t section, int *malloc_error);

#endif

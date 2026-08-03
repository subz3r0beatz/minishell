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

typedef struct s_robin	t_robin;

int		build_prompt(t_robin *env, char **prompt);
char	*get_username(t_robin *env, char *buffer, int *no_malloc_error);
char	*get_hostname(t_robin *env, char *buffer, int *no_malloc_error);
char	*get_prompt_pwd(t_robin *env, char *buffer, int *no_malloc_error);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   environment.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 23:53:20 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/29 16:31:39 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENVIRONMENT_H
# define ENVIRONMENT_H

# include "build_env.h"

typedef struct s_env
{
	char			*key;
	char			*value;
	int				is_exported;
}				t_env;

t_env			*create_env_node(char *key, char *value, int is_exported);
t_robin_node	create_node(t_robin *env,
					char *key, char *value, int is_exported);
int				delete_node(void *key, void *value);
void			free_matrix(char **matrix, size_t size);
char			**env_to_matrix(t_minishell *shell, char **modif);

#endif

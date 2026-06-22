/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   environment.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 23:53:20 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/18 18:00:30 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENVIRONMENT_H
# define ENVIRONMENT_H

typedef struct s_env
{
	char			*key;
	char			*value;
	int				is_exported;
}				t_env;

t_env			*create_env_node(char *key, char *value, int is_exported);
t_robin			*build_env(char **envp);
t_robin_node	create_node(t_robin *env,
					char *key, char *value, int is_exported);

#endif

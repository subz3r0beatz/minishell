/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_pwd.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 01:34:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 11:04:44 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*get_raw_cwd(t_robin *env, char *buffer)
{
	t_robin_node	*node;
	char			*pwd;

	pwd = NULL;
	node = robin_search(env, "PWD");
	if (node && node->value && ((t_env *)node->value)->value)
		pwd = ft_strdup(((t_env *)node->value)->value);
	else if (getcwd(buffer, 8192))
		pwd = ft_strdup(buffer);
	else
	{
		ft_putstr_fd("minishell: pwd: error retrieving current directory: ", 2);
		ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
		ft_putendl_fd(strerror(errno), 2);
		return (NULL);
	}
	if (!pwd)
		return (NULL);
	return (pwd);
}

char	*get_pwd(t_robin *env, char *buffer)
{
	t_robin_node	*node;
	char			*pwd;
	char			*home;
	size_t			home_len;

	pwd = get_raw_cwd(env, buffer);
	if (!pwd)
		return (NULL);
	node = robin_search(env, "HOME");
	if (!node || !node->value || !((t_env *)node->value)->value)
		return (pwd);
	home = ((t_env *)node->value)->value;
	home_len = ft_strlen(home);
	if (ft_strncmp(home, pwd, home_len) == 0
		&& (pwd[home_len] == '/' || !pwd[home_len]))
	{
		home = ft_strjoin("~", &pwd[home_len]);
		free(pwd);
		pwd = home;
	}
	return (pwd);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_prompt_pwd.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 01:34:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/03 03:31:11 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*get_raw_cwd(t_robin *env, char *buffer, int *no_malloc_error)
{
	t_robin_node	*node;
	char			*pwd;

	pwd = NULL;
	node = robin_search(env, "PWD");
	if (node && node->value && ((t_env *)node->value)->value)
		pwd = ft_strdup(((t_env *)node->value)->value);
	else if (getcwd(buffer, PATH_MAX))
		pwd = ft_strdup(buffer);
	else
	{
		perror("minishell: pwd: error retrieving current directory: "
			"getcwd: cannot access parent directories");
		return (NULL);
	}
	if (!pwd)
	{
		*no_malloc_error = 0;
		return (NULL);
	}
	return (pwd);
}

char	*get_prompt_pwd(t_robin *env, char *buffer, int *no_malloc_error)
{
	t_robin_node	*node;
	char			*pwd;
	char			*home;
	size_t			home_len;

	pwd = get_raw_cwd(env, buffer, no_malloc_error);
	if (!pwd)
		return (NULL);
	node = robin_search(env, "HOME");
	if (!node || !node->value || !((t_env *)node->value)->value)
		return (pwd);
	home = ((t_env *)node->value)->value;
	home_len = ft_strlen(home);
	if (home_len && home[home_len - 1] == '/')
		home_len--;
	if (ft_strncmp(home, pwd, home_len) == 0
		&& (pwd[home_len] == '/' || !pwd[home_len]))
	{
		home = ft_strjoin("~", &pwd[home_len]);
		free(pwd);
		pwd = home;
		if (!pwd)
			*no_malloc_error = 0;
	}
	return (pwd);
}

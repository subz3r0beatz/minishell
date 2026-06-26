/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_pwd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 10:09:53 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 18:58:53 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	insert_pwd(t_robin *env, char *pwd, char *value)
{
	t_robin_node	robin_node;

	if (!pwd || !value)
	{
		free(pwd);
		free(value);
		return (1);
	}
	robin_node = create_node(env, pwd, value, 1);
	if (!robin_node.key || !robin_node.value)
		return (1);
	if (robin_insert(env, robin_node))
	{
		env->del_function(robin_node.key, robin_node.value);
		return (1);
	}
	return (0);
}

int	handle_pwd(t_robin *env)
{
	char			*pwd;
	char			*value;
	char			pwd_buf[8192];
	t_robin_node	*robin_node;

	robin_node = robin_search(env, "PWD");
	if (robin_node)
		return (0);
	if (!getcwd(pwd_buf, 8192))
	{
		ft_putstr_fd("minishell: pwd: error retrieving current directory: ", 2);
		ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
		ft_putendl_fd(strerror(errno), 2);
		return (1);
	}
	pwd = ft_strdup("PWD");
	value = ft_strdup(pwd_buf);
	return (insert_pwd(env, pwd, value));
}

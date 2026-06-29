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

static int	insert_pwd(t_minishell *shell, char *pwd, char *value)
{
	t_robin_node	robin_node;

	robin_node = create_node(shell->env, pwd, value, 1);
	if (!robin_node.key || !robin_node.value)
		return (1);
	if (robin_insert(shell->env, robin_node))
	{
		shell->env->del_function(robin_node.key, robin_node.value);
		return (1);
	}
	shell->exported_count++;
	return (0);
}

int	handle_pwd(t_minishell *shell)
{
	char					*pwd;
	char					*value;
	char					pwd_buf[8192];

	if (robin_search(shell->env, "PWD"))
		return (0);
	if (!getcwd(pwd_buf, 8192))
	{
		perror("minishell: pwd: error retrieving current directory: "
				 "getcwd cannot access parent directories: ");
		return (1);
	}
	pwd = ft_strdup("PWD");
	value = ft_strdup(pwd_buf);
	if (!pwd || !value)
	{
		free(pwd);
		free(value);
		return (1);
	}
	if (insert_pwd(shell, pwd, value))
		return (1);
	return (0);
}

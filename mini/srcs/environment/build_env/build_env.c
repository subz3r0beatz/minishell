/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 18:17:23 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 14:38:08 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	get_capacity(char **envp)
{
	size_t	i;
	size_t	cap;

	i = 0;
	cap = 4096;
	while (envp && envp[i] && cap < SIZE_MAX / 2)
	{
		if (cap < i)
			cap *= 2;
		i++;
	}
	if (cap < i)
		return (0);
	return (cap);
}

static int	parse_var(t_minishell *shell, char *str)
{
	size_t	i;
	char	saved;
	char	*value;

	value = NULL;
	i = 0;
	while (str[i] && str[i] != '=')
		i++;
	if (str[i] && str[i + 1])
		value = &str[i + 1];
	saved = str[i];
	str[i] = '\0';
	if (insert_new_node(shell, str, value, 1))
	{
		str[i] = saved;
		return (1);
	}
	str[i] = saved;
	return (0);
}

static int	cpy_env(t_minishell *shell, char **envp)
{
	size_t	i;
	size_t	cap;

	cap = get_capacity(envp);
	if (!cap)
		return (0);
	shell->env = robin_init(cap, fnv1a, ft_strcmp, delete_node);
	if (!shell->env)
		return (1);
	i = 0;
	while (envp && envp[i])
	{
		if (parse_var(shell, envp[i]))
			return (1);
		i++;
	}
	return (0);
}

int	build_env(t_minishell *shell, char **envp)
{
	if (cpy_env(shell, envp))
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	if (!shell->env)
		return (1);
	if (handle_pwd(shell) || handle_oldpwd(shell)
		|| handle_shlvl(shell) || handle_underscore(shell)
		|| handle_pid(shell))
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	return (0);
}

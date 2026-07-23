/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unexport_vars.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 15:54:38 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/23 19:59:55 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	is_valid_key(char *str, int *append)
{
	size_t	i;

	*append = 0;
	if (ft_isalpha(str[0]) || str[0] == '_')
	{
		i = 1;
		while (str[i] && str[i] != '=')
		{
			if (!ft_isalnum(str[i]) && str[i] != '_')
				break ;
			i++;
		}
		if (str[i] == '=' || !str[i])
			return (i);
		if (str[i] == '+' && str[i + 1] == '=')
		{
			*append = 1;
			return (i + 1);
		}
	}
	ft_putstr_fd("minishell: export: `", STDERR_FILENO);
	ft_putstr_fd(str, STDERR_FILENO);
	ft_putstr_fd("': not a valid identifier\n", STDERR_FILENO);
	return (0);
}

static int	lookup_var(t_minishell *shell, char *key, char *value)
{
	t_robin_node	*robin_node;
	int				has_value;

	robin_node = robin_search(shell->env, key);
	if (!robin_node)
		return (1);
	free(key);
	has_value = 0;
	if (((t_env *)robin_node->value)->value)
		has_value = 1;
	if (value)
	{
		free(((t_env *)robin_node->value)->value);
		((t_env *)robin_node->value)->value = value;
		if (((t_env *)robin_node->value)->is_exported && has_value)
			shell->exported_count--;
	}
	((t_env *)robin_node->value)->is_exported = 0;
	return (0);
}

static int	insert_new_var(t_minishell *shell, char *key, char *value)
{
	t_robin_node	robin_node;

	robin_node = create_node(shell->env, key, value, 0);
	if (!robin_node.key || !robin_node.value)
		return (2);
	if (robin_insert(shell->env, robin_node))
	{
		shell->env->del_function(robin_node.key, robin_node.value);
		return (2);
	}
	return (0);
}

static int	insert_var(t_minishell *shell, char *str)
{
	size_t			i;
	char			*key;
	char			*value;
	char			*oldvalue;
	int				append;

	i = is_valid_key(str, &append);
	if (i == 0)
		return (1);
	key = ft_substr(str, 0, i);
	if (!key)
		return (2);
	get_var_value(shell, key, &oldvalue);
	value = NULL;
	if (str[i] && !append)
		value = ft_strdup(str + i + 1);
	else if (append && (oldvalue || str[i]))
		value = ft_strjoin(oldvalue, str + i + 1);
	if ((str[i] || append) && !value)
		free(key);
	if ((str[i] || append) && !value)
		return (2);
	if (!lookup_var(shell, key, value))
		return (0);
	return (insert_new_var(shell, key, value));
}

int	unexport_vars(t_minishell *shell, char **args)
{
	int		status;
	int		ret;
	size_t	i;

	ret = 0;
	i = 0;
	while (args[i])
	{
		status = insert_var(shell, args[i]);
		if (status == 1)
			ret = 1;
		if (status == 2)
		{
			ft_putstr_fd("minishell: export: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
			return (status);
		}
		i++;
	}
	return (ret);
}

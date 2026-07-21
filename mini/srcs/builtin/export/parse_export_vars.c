/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_export_vars.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 15:31:03 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/19 14:20:35 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	is_valid_key(char *str)
{
	size_t	i;

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
			return (i + 1);
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
	}
	if (!((t_env *)robin_node->value)->is_exported
		&& ((value && !has_value) || (!value && has_value)))
		shell->exported_count++;
	((t_env *)robin_node->value)->is_exported = 1;
	return (0);
}

static int	insert_new_var(t_minishell *shell, char *key, char *value)
{
	t_robin_node	robin_node;

	robin_node = create_node(shell->env, key, value, 1);
	if (!robin_node.key || !robin_node.value)
		return (2);
	if (robin_insert(shell->env, robin_node))
	{
		shell->env->del_function(robin_node.key, robin_node.value);
		return (2);
	}
	if (value)
		shell->exported_count++;
	return (0);
}

static int	insert_var(t_minishell *shell, char *str)
{
	size_t			i;
	char			*key;
	char			*value;

	i = is_valid_key(str);
	if (!i)
		return (1);
	key = ft_substr(str, 0, i);
	value = NULL;
	if (str[i])
		value = ft_strdup(str + i + 1);
	if (!key || (str[i] && !value))
	{
		free(key);
		free(value);
		return (2);
	}
	if (!lookup_var(shell, key, value))
		return (0);
	return (insert_new_var(shell, key, value));
}

int	parse_export_vars(t_minishell *shell, char **args)
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
			return (2);
		}
		i++;
	}
	return (ret);
}

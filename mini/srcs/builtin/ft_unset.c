/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_unset.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 07:09:11 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/19 14:36:02 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	usage_error(char c)
{
	ft_putstr_fd("minishell: unset: -", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd(": invalid option\n"
		"unset: usage: unset [-v] [name ...]\n", STDERR_FILENO);
	return (0);
}

static size_t	check_flags(char **args, int *vars)
{
	size_t	i;
	size_t	j;

	i = 1;
	while (args[i] && args[i][0] == '-' && args[i][1] != '\0')
	{
		if (args[i][1] == '-' && args[i][2] == '\0')
			return (i + 1);
		j = 0;
		while (args[i][++j])
		{
			if (args[i][j] == 'v')
				*vars = 1;
			else
				return (usage_error(args[i][j]));
		}
		i++;
	}
	return (i);
}

static int	is_valid_key(char *str)
{
	size_t	i;

	if (ft_isalpha(str[0]) || str[0] == '_')
	{
		i = 1;
		while (str[i])
		{
			if (!ft_isalnum(str[i]) && str[i] != '_')
				break ;
			i++;
		}
		if (str[i] == '\0')
			return (1);
	}
	ft_putstr_fd("minishell: unset: `", STDERR_FILENO);
	ft_putstr_fd(str, STDERR_FILENO);
	ft_putstr_fd("': not a valid identifier\n", STDERR_FILENO);
	return (0);
}

static int	parse_vars(t_minishell *shell, char **args)
{
	int				status;
	int				has_value;
	size_t			i;
	t_robin_node	*robin_node;

	status = 0;
	i = 0;
	while (args[i])
	{
		if (is_valid_key(args[i]))
		{
			has_value = 0;
			robin_node = robin_search(shell->env, args[i]);
			if (robin_node && (((t_env *)robin_node->value)->is_exported
					&& ((t_env *)robin_node->value)->value))
				has_value = 1;
			if (!robin_remove(shell->env, args[i]))
				if (has_value)
					shell->exported_count--;
		}
		else
			status = 1;
		i++;
	}
	return (status);
}

int	ft_unset(t_minishell *shell, char **args, int fd_out)
{
	size_t	i;
	int		status;

	(void) fd_out;
	if (!args[1])
		return (0);
	i = check_flags(args, &status);
	if (i == 0)
		return (2);
	status = parse_vars(shell, &args[i]);
	ft_free_matrix(shell->exported,
		ft_memlen(shell->exported, sizeof(char *)));
	shell->exported = NULL;
	return (status);
}

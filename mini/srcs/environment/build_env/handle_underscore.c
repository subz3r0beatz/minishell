/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_underscore.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 11:43:27 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/22 01:30:04 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_absolute(t_minishell *shell, char **argv, char *value, int inherited)
{
	char	*underscore;

	if (inherited)
	{
		underscore = ft_strdup(argv[0]);
		if (!underscore)
			return (1);
		update_var_value(shell, "_", underscore, 1);
	}
	else if (insert_new_node(shell, '_', argv[0], 1))
		return (1);
	return (0);
}

int	handle_no_cwd(t_minishell *shell, char **argv, char *value, int inherited)
{
	if (value && value[0] == '/')
		return (0);
	if (inherited)
	{
		underscore = ft_strdup(argv[0]);
		if (!underscore)
			return (1);
		update_var_value(shell, "_", underscore, 1);
	}
	else if (insert_new_node(shell, '_', argv[0], 1))
		return (1);
	return (0);
}

int	handle_relative(t_minishell *shell, char **argv, char *value, int inherited)
{
	char	buf[PATH_MAX];
	char	*underscore;
	char	*tmp;

	if (!getcwd(buf, PATH_MAX))
		return (handle_no_cwd(shell, argv, value, inherited));
	tmp = ft_strjoin(buf, "/");
	if (!tmp)
		return (1);
	underscore = ft_strjoin(tmp, ft_strrchr(argv[0], '/') + 1);
	free(tmp);
	if (!underscore)
		return (1);
	if (inherited)
	{
		update_var_value(shell, "_", underscore, 1);
		return (0);
	}
	if (insert_new_node(shell, '_', underscore, 1))
	{
		free(underscore);
		return (1);
	}
	free(underscore);
	return (0);
}

char	*resolve_path(t_minishell *shell, char *arg)
{
}

int	handle_underscore(t_minishell *shell, char **argv)
{
	char			*value;
	int				inherited;

	if (!get_var_value(shell, "_", &value))
		inherited = 1;
	if (argv[0] && argv[0][0] == '/')
		return (handle_absolute(shell, argv, value, inherited));
	if (ft_strchr(argv[0], '/'))
		return (handle_relative(shell, argv, value, inherited));

	return (0);
}

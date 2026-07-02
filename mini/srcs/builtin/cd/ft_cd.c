/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 18:33:00 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/02 19:42:53 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	usage_error(char c)
{
	ft_putstr_fd("minishell: cd: -", STDERR_FILENO);
	ft_putchar_fd(c, STDERR_FILENO);
	ft_putstr_fd(": invalid option\n"
		"cd: usage: cd [-L|[-P [-e]]] [dir]\n", STDERR_FILENO);
	return (0);
}

static int	check_flags(char **args, int *logical, int *silent)
{
	size_t	i;
	size_t	j;

	*logical = 1;
	*silent = 0;
	i = 0;
	while (args[++i] && args[i][0] == '-' && args[i][1])
	{
		if (args[i][1] == '-' && !args[i][2])
			return (i + 1);
		j = 0;
		while (args[i][++j])
		{
			if (args[i][j] == 'L')
				*logical = 1;
			else if (args[i][j] == 'P')
				*logical = 0;
			else if (args[i][j] == 'e')
				*silent = 1;
			else
				return (usage_error(args[i][j]));
		}
	}
	return (i);
}

static char	*parse_dir(t_minishell *shell, char *dir_arg)
{
	t_robin_node	*robin_node;

	if (!dir_arg)
	{
		robin_node = robin_search(shell->env, "HOME");
		if (!robin_node || !((t_env *)robin_node->value)->value)
		{
			ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
			return (NULL);
		}
		return (((t_env *)robin_node->value)->value);
	}
	else if (dir_arg[0] == '-' && dir_arg[1] == '\0')
	{
		robin_node = robin_search(shell->env, "OLDPWD");
		if (!robin_node || !((t_env *)robin_node->value)->value)
		{
			ft_putstr_fd("minishell: cd: OLDPWD not set\n", STDERR_FILENO);
			return (NULL);
		}
		return (((t_env *)robin_node->value)->value);
	}
	return (dir_arg);
}

static char	*canonalize_path(t_minishell *shell, char *pwd, char *path)
{
	char	*ret;
	size_t	i;
	size_t	j;

	ret = ft_strjoin(pwd, path);
	free(path);
	if (!ret)
		return (NULL);
	i = 0;
	while (ret[i])
	{
		if (ret[i] == '/' && ret[i + 1] == '/')
			while (ret[i] && ret[i + 1] == '/')
				ft_shift_left(ret, 1);
		if (ret[i] == '.' && ret[i + 1] == '.')
		{
			j = 0;
			while (ret[i] && ret[i] != '/')
			{
				i--;
				j++;
			}
			ft_shift_left(ret, j + 2);
		}
		i++;
	}
	return (path);
}

static int	do_chdir(t_minishell *shell, char *path, int logical, int fd_out)
{
	char	*pwd;
	char	buffer[PATH_MAX];

	pwd = NULL;
	if (get_var_value(shell->env, "PWD", pwd))
	{
		if (!getcwd(buffer, PATH_MAX))
		{
			perror("minishell: cd: error retrieving current directory: "
				"getcwd: cannot access parent directories");
			return (1);
		}
		pwd = ft_strdup(buffer);
		if (!pwd)
			return (1);
	}
	if (logical && path[0] != '/')
		path = canonalize_path(shell, path, pwd);
	if (!path)
		return (1);
	if (!chdir(path))
	{
		if (!get_var_value(shell->env, "PWD", pwd))
			update_var_value(shell->env, "OLDPWD", pwd);
		update_var_value(shell->env, "PWD", path);
		return (0);
	}
	ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
	perror(path);
	return (1);
}

int	ft_cd(t_minishell *shell, char **args, int fd_out)
{
	char	*path;
	size_t	i;
	int		logical;
	int		silent;
	int		is_dash;

	is_dash = 0;
	i = check_flags(args, &logical, &silent);
	if (i == 0)
		return (2);
	if (args[i] && args[i + 1])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", STDERR_FILENO);
		return (1);
	}
	if (args[i][0] == '-' && !args[i][1])
		is_dash = 1;
	path = parse_dir(shell, args[i]);
	if (!path)
		return (1);
	if (do_chdir(shell, path, logical, fd_out))
		return (1);
	if (is_dash)
		ft_putendl_fd(path, fd_out);
	return (0);
}

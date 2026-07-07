#include "minishell.h"

static int	handle_env_paths(t_minishell *shell, char *arg, char **dir, int *print_path)
{
	char	*env_value;

	if (!arg)
	{
		if (get_var_value(shell->env, "HOME", &env_value) || !*dir)
		{
			ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
			return (1);
		}
		*dir = ft_strdup(env_value);
	}
	else if (arg[0] == '-' && !arg[1])
	{
		if (get_var_value(shell->env, "OLDPWD", &env_value) || !*dir)
		{
			ft_putstr_fd("minishell: cd: OLDPWD not set\n", STDERR_FILENO);
			return (1);
		}
		*dir = ft_strdup(env_value);
		*print_path = 1;
	}
	return (0);
}

static char	*build_full_path(char *base, char *target)
{
	char	*tmp;
	char	*path;

	tmp = ft_strjoin(base, '/');
	if (!tmp)
		return (NULL);
	path = ft_strjoin(tmp, target);
	free(tmp);
	return (path);
}

static int	compare_paths(char **paths, char *arg, char **dir, int *print_path)
{
	char		*path;
	size_t	i;

	i = 0;
	while (paths[i])
	{
		path = build_full_path(paths[i], arg);
		if (!path)
		{
			ft_putstr_fd("minishell: cd: malloc "
				"cannot allocate memory\n", STDERR_FILENO);
			return (1);
		}
		if (access(path, F_OK) == 0)
		{
			*print_path = 1;
			*dir = path;
			return (0);
		}
		free(path);
		i++;
	}
	return (0);
}

static int	parse_cdpath(t_minishell *shell, char *arg, char **dir, int *print_path)
{
	char	*cdpath;
	char	**paths;
	int		ret;

	if (arg[0] == '/' || !ft_strncmp(arg, "./", 2) || !ft_strncmp(arg, "../", 3)
		|| !ft_strcmp(arg, ".") || !ft_strcmp(arg, "..")
		|| get_var_value(shell->env, "CDPATH", &cdpath) || !cdpath || !*cdpath)
	{
		*dir = ft_strdup(arg);
		return (0);
	}
	paths = ft_split(cdpath, ':');
	if (!paths)
	{
		ft_putstr_fd("minishell: cd: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	ret = compare_paths(paths, arg, dir, print_path);
	ft_free_matrix(paths, ft_memlen(paths, sizeof(char *)));
	if (!ret && !*dir)
		dir = ft_strdup(arg);
	return (ret);
}

int	parse_dir(t_minishell *shell, char *arg, char **dir, int	*print_path)
{
	char	*env_value;

	*dir = NULL;
	*print_path = 0;
	if (!arg || (arg[0] == '-' && !arg[1]))
	{
		if (handle_env_paths(shell, arg, dir, print_path))
			return (1);
	}
	else if (parse_cdpath(shell, arg, dir, print_path))
		return (1);
	if (!*dir)
	{
		ft_putstr_fd("minishell: cd: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	return (0);
}

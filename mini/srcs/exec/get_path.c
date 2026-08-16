#include "minishell.h"

static char	*malloc_error(void)
{
	ft_putstr_fd("minishell: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (NULL);
}

static char	*command_error(char *cmd, int exists, int is_dir)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(cmd, STDERR_FILENO);
	if (is_dir)
		ft_putstr_fd(": Is a directory\n", STDERR_FILENO);
	else if (exists == 2)
		ft_putstr_fd(": Permission denied\n", STDERR_FILENO);
	else
		ft_putstr_fd(": command not found\n", STDERR_FILENO);
	return (NULL);
}

static char	*check_path(char **split, char *cmd, int *exists)
{
	char		*path;
	size_t		i;
	struct stat	st;

	i = 0;
	while (split[i])
	{
		path = ft_strjoin_3(split[i], "/", cmd);
		if (!path)
			return (malloc_error());
		if (stat(path, &st) == 0 && !S_ISDIR(st.st_mode))
		{
			*exists = 2;
			if (access(path, X_OK) == 0)
				return (path);
		}
		free(path);
		i++;
	}
	return (command_error(cmd, *exists, 0));
}

char	*get_path(t_minishell *shell, char *cmd, int *exists, int *is_dir)
{
	char	*path;
	char	**split;
	struct stat	st;

	if (!cmd || !cmd[0])
		return (command_error(cmd, 0, 0));
	if (ft_strchr(cmd, '/'))
	{
		if (stat(cmd, &st) == 0 && S_ISDIR(st.st_mode))
		{
			*is_dir = 1;
			return (command_error(cmd, 2, 1));
		}
		path = ft_strdup(cmd);
		if (!path)
			return (malloc_error());
		return (path);
	}
	get_var_value(shell, "PATH", &path);
	if (path && path[0] && check_exported(shell, "PATH"))
	{
		split = ft_split(path, ':');
		if (!split)
			return (malloc_error());
		path = check_path(split, cmd, exists);
		ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
		return (path);
	}
	path = ft_strdup(cmd);
	if (!path)
		return (malloc_error());
	return (path);
}

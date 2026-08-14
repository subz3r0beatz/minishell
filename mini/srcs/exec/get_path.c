#include "minishell.h"

static char	*malloc_error(void)
{
	ft_putstr_fd("minishell: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (NULL);
}

static char	*command_error(char *cmd, int exists)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(cmd, STDERR_FILENO);
	if (exists)
		ft_putstr_fd(": command not found\n", STDERR_FILENO);
	else
		ft_putstr_fd(": Permission denied\n", STDERR_FILENO);
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
		if (stat(path, &st) == 0)
		{
			if (S_ISDIR(st.st_mode))
			{
				free(path);
				i++;
				continue ;
			}
			*exists = 1;
			if (access(path, X_OK) == 0)
				return (path);
		}
		free(path);
		i++;
	}
	return (command_error(cmd, *exists));
}

char	*get_path(t_minishell *shell, char *cmd, int *exists)
{
	char	*path;
	char	**split;

	if (ft_strchr(cmd, '/'))
	{
		path = ft_strdup(cmd);
		if (!path)
			return (malloc_error());
		return (path);
	}
	get_var_value(shell, "PATH", &path);
	if (path && path[0] && check_exported(shell, "PATH"))
	{
		*exists = 1;
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

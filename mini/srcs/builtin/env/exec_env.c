#include "minishell.h"
#include <stdlib.h>

static void	switch_dir(char	*path)
{
	if (path && chdir(path))
	{
		ft_putstr_fd("minishell: env: cannot change directory to '", STDERR_FILENO);
		ft_putstr_fd(path, STDERR_FILENO);
		ft_putstr_fd("'", STDERR_FILENO);
		perror();
		exit(125);
	}
}

static char	*malloc_error(void)
{
	ft_putstr_fd("minishell: env: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (NULL);
}

static char	*check_access(char *targets[4], char *path)
{
	char	*tmp;
	size_t	i;

	i = 0;
	while (targets[i])
	{
		if (access(targets[i], F_OK) == 0)
		{
			if (!path)
				path = ft_strdup(targets[i]);
			else
			{
				tmp = ft_strjoin(path, ":");
				free(path);
				if (!tmp)
					return (malloc_error());
				path = ft_strjoin(tmp, targets[i]);
				free(tmp);
			}
			if (!path)
				return (malloc_error());
		}
		i++;
	}
	return (path);
}

static char	*get_path(char **exported)
{
	char	*targets[4];
	char	*path;
	size_t	i;

	i = 0;
	while (exported[i])
	{
		if (!ft_strncmp(exported[i], "PATH=", 5))
			return (&exported[i][5]);
		i++;
	}
	targets[0] = "/bin";
	targets[1] = "/usr/bin";
	targets[2] = "/usr/local/bin";
	targets[3] = "/opt/homebrew/bin";
	path = NULL;
	path = check_access(targets, path);
	if (!path)
		path = ft_strdup("/bin:/usr/bin");
	if (!path)
		return (malloc_error());
	return (path);
}

static char	*resolve_cmd_path(char **args, char **exported, t_flags *flags)
{
	char	*cmd;
	char	*path;
	char	**split;

	cmd = args[0];
	if (ft_strchr(cmd, '/'))
		return (ft_strdup(cmd));
	path = get_path(exported);
	if (!path)
		exit(125);
	if (!path[0])
		return (ft_strdup(cmd));
	split = ft_split(path, ':');

}

static void	exec_cmd(char **args, char **exported, t_flags *flags)
{

}

int	exec_env(char	**args, char **exported, t_flags *flags)
{
	pid_t	pid;
	int		status;
	
	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd("minishell: env: fork failed\n, 2");
		return (125);
	}
	if (pid == 0)
	{
		switch_dir(flags->chdir_path);
		exec_cmd(args, exported, flags);
	}
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		return (WIFEXITED(status));
	if (WIFSIGNALED(statui))
		return (128 + WTERMSIG(status));
	return (0);
}


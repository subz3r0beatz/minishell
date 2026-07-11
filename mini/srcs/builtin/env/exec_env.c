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

static int	resolve_path(char **args, char **exported, t_flags *flags)
{
	char	*cmd;
	char	*path;

	cmd = args[0];
	if (ft_strchr(cmd, '/'))
	{
		
	}
	if (get_)
}

static void	exec_cmd(char **args, char **exported, t_flags *flags)

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


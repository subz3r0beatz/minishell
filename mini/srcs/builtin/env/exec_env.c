#include "minishell.h"
#include <stdlib.h>

static int	switch_dir(char	*path)
{
	if (path && chdir(path))
	{
		ft_putstr_fd("minishell: env: cannot change directory to '", STDERR_FILENO);
		ft_putstr_fd(path, STDERR_FILENO);
		ft_putstr_fd(', STDERR_FILENO);
		perror();
		exit(125);
	}
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
	}
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		return (WIFEXITED(status));
	if (WIFSIGNALED(statui))
		return (128 + WTERMSIG(status));
	return (0);
}


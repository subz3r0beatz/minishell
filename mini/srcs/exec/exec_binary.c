#include "minishell.h"
#include <string.h>

static int	wait_exec(t_minishell *shell, pid_t pid)
{
	int	status;

	waitpid(pid, &status, 0);
	init_interactive_signals(1);
	if (WIFEXITED(status))
		shell->exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
	{
		shell->exit_status = 128 + WTERMSIG(status);
		if (shell->exit_status - 128 == SIGINT)
			ft_putstr_fd("\n", STDERR_FILENO);
		if (shell->exit_status - 128 == SIGQUIT)
			ft_putstr_fd("Quit\n", STDERR_FILENO);
	}
	return (shell->exit_status);
}

static void	do_execv(t_minishell *shell, t_ast_node *node, char *path)
{
	int		err;

	if (!shell->exported)
		shell->exported = env_to_matrix(shell);
	if (!shell->exported)
	{
		free(path);
		ft_putstr_fd("minishell: exec: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		exit_shell(shell, 1);
	}
	execve(path, node->args, shell->exported);
	err = errno;
	free(path);
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(node->args[0], STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putendl_fd(strerror(err), STDERR_FILENO);
	if (err == ENOENT)
		exit_shell(shell, 127);
	exit_shell(shell, 126);
}

static void	check_builtin(t_minishell *shell, t_ast_node *node)
{
	int	builtin;
	int	status;

	builtin = is_builtin(node->args[0]);
	if (builtin)
	{
		status = shell->builtin_func_table[builtin - 1](shell, node->args);
		if (node->redir)
		{
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
		}
		exit_shell(shell, status);
	}
}

static void	exec_binary_child(t_minishell *shell, t_ast_node *node)
{
	char	*path;
	int		exists;

	init_ignore_signals(0);
	if (apply_redirections(shell, node->redir))
		exit_shell(shell, 1);
	check_builtin(shell, node);
	exists = 0;
	path = get_path(shell, node->args[0], &exists);
	if (!path)
	{
		if (exists == 1)
			exit_shell(shell, 127);
		else if (exists == 2)
			exit_shell(shell, 126);
		exit_shell(shell, 1);
	}
	do_execv(shell, node, path);
}

int	exec_binary(t_minishell *shell, t_ast_node *node)
{
	pid_t	pid;

	init_ignore_signals(1);
	pid = fork();
	if (pid < 0)
	{
		init_interactive_signals(1);
		ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
		shell->exit_status = 1;
		return (1);
	}
	if (pid == 0)
		exec_binary_child(shell, node);
	return (wait_exec(shell, pid));
}

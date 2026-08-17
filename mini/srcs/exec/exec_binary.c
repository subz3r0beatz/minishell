#include "minishell.h"

static int	wait_exec(t_minishell *shell, pid_t pid)
{
	int	status;

	waitpid(pid, &status, 0);
	init_interactive_signals();
	if (WIFEXITED(status))
		shell->exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
	{
		shell->exit_status = 128 + WTERMSIG(status);
		if (WTERMSIG(status) == SIGINT)
			g_signal_status = 130;
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

	builtin = is_builtin(node->args[0]);
	if (builtin)
	{
		shell->exit_status = shell->builtin_func_table[builtin - 1](shell, node->args);
		exit_shell(shell, shell->exit_status);
	}
}

static void	exec_binary_child(t_minishell *shell, t_ast_node *node)
{
	char	*path;
	int		exists;
	int		is_dir;

	shell->is_child = 1;
	init_ignore_signals(0);
	if (redirections(node->redir))
		exit_shell(shell, 1);
	check_builtin(shell, node);
	exists = 0;
	is_dir = 0;
	path = get_path(shell, node->args[0], &exists, &is_dir);
	if (!path)
	{
		if (is_dir || exists == 2)
			exit_shell(shell, 126);
		exit_shell(shell, 127);
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
		init_interactive_signals();
		ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
		return (1);
	}
	if (pid == 0)
		exec_binary_child(shell, node);
	return (wait_exec(shell, pid));
}

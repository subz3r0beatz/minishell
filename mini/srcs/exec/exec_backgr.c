#include "minishell.h"

int	exec_backgr(t_minishell *shell, t_ast_node *node, char *argv0)
{
	pid_t	pid;

	if (!node)
		return (shell->exit_status);
	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
		shell->exit_status = 1;
		return (1);
	}
	if (pid == 0)
	{
		init_child_signals();
		exit_shell(shell, NULL, 0, exec(shell, node->left, argv0));
	}
	shell->exit_status = 0;
	if (node->right)
		return (exec(shell, node->right, argv0));
	return (0);
}

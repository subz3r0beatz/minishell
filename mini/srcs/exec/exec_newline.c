#include "minishell.h"

int	exec_newline(t_minishell *shell, t_ast_node *node)
{
	int	status;

	status = exec(shell, node->left);
	if (node->right)
	{
		if (g_signal_status == 130)
			ft_putchar_fd('\n', STDOUT_FILENO);
		status = exec(shell, node->right);
	}
	return (status);
}

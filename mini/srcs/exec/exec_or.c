#include "minishell.h"

int	exec_or(t_minishell *shell, t_ast_node *node, char *argv0)
{
	int	status;

	if (!node)
		return (shell->exit_status);
	status = exec(shell, node->left, argv0);
	if (status != 0)
		return (exec(shell, node->right, argv0));
	return (status);
}

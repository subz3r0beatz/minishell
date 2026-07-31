#include "minishell.h"

int	exec_semi(t_minishell *shell, t_ast_node *node, char *argv0)
{
	if (!node)
		return (shell->exit_status);
	exec(shell, node->left, argv0);
	if (node->right)
		return (exec(shell, node->right, argv0));
	return (shell->exit_status);
}

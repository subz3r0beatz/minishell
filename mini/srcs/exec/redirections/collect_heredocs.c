#include "minishell.h"

int	collect_heredocs(t_minishell *shell, t_ast_node *node)
{
	int		sigint_status;
	t_redir *redir;

	if (!node)
		return (0);
	if (node->type == NODE_CMD)
	{
		redir = node->redir;
		while (redir)
		{
			if (redir->type == TOKEN_DLESS)
			{
				sigint_status = 0;
				redir->fd = handle_heredoc(shell, redir->file, &sigint_status);
				if (sigint_status == 1)
					shell->exit_status = 130;
				if (redir->fd < 0 || sigint_status)
					return (1);
			}
			redir = redir->next;
		}
	}
	if (collect_heredocs(shell, node->left))
		return (1);
	if (collect_heredocs(shell, node->right))
		return (1);
	return (0);
}

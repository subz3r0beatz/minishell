#include "minishell.h"

int	check_exported(t_minishell *shell, char *key)
{
	t_robin_node	*node;

	node = robin_search(shell->env, key);
	if (!node)
		return (0);
	if (((t_env *)node->value)->value && ((t_env *)node->value)->is_exported)
		return (1);
	return (0);
}

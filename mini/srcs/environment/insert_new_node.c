#include "minishell.h"

int	insert_new_node(t_minishell *shell, char *key,
	char *value, int is_exported)
{
	t_robin_node	node;
	char			*key_dup;
	char			*value_dup;

	key_dup = ft_strdup(key);
	if (!key_dup)
		return (1);
	value_dup = NULL;
	if (value)
		value_dup = ft_strdup(value);
	if (value && !value_dup)
	{
		free(key_dup);
		return (1);
	}
	node = create_node(shell->env, key_dup, value_dup, is_exported);
	if (!node.key || !node.value)
		return (1);
	if (robin_insert(shell->env, node))
		return (1);
	if (value && is_exported)
		shell->exported_count++;
	return (0);
}

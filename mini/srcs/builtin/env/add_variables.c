#include "minishell.h"
#include <stddef.h>

static int	lookup_var(char **exported, char *arg, char *ptr, size_t len)
{
	size_t	i;

	i = 0;
	while (exported[i])
	{
		if (!ft_strncmp(exported[i], arg, arg - ptr))
		{
			free(exported[i]);
			exported[i] = ft_strdup(arg);
			if (!exported[j])
			{
				ft_putstr_fd("minishell: env: malloc: "
					"cannot allocate memory\n", STDERR_FILENO);
				ft_free_matrix(exported, len);
				return (1);
			}
		}
		i++;
	}
		return (0);
}

static int	add_var(char **exported, char *arg, size_t len)
{
	char	**new_exported;

	new_exported = ft_realloc(exported, len + 2, sizeof(char *));
	if (!new_exported)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		ft_free_matrix(exported, len);
		return (1);
	}
	new_exported[len] = ft_strdup(arg);
	if (!new_exported[len])
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		ft_free_matrix(exported, len + 1);
		return (1);
	}
	exported = new_exported;
	return (0);
}

size_t	add_variables(char **args, char **exported, size_t i)
{
	char	*ptr;
	size_t	j;
	size_t	len;
	int		status;

	len = ft_memlen(exported, sizeof(char *));
	while (args[i])
	{
		ptr = ft_strchr(args[i], '=');
		if (!ptr)
			return (i);
		status = lookup_var(exported, args[i], ptr, len);
		if (!status)
			return (0);
		if (status == 2)
		{
			if (add_var(exported, args[i], len))
				return (0);
			len++;
		}
		i++;
	}
	return (i);
}

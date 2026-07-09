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
			if (!exported[i])
			{
				ft_putstr_fd("minishell: env: malloc: "
					"cannot allocate memory\n", STDERR_FILENO);
				ft_free_matrix(exported, len);
				return (0);
			}
			return (1);
		}
		i++;
	}
	return (2);
}

static int	add_var(char ***exported, char *arg, size_t len)
{
	char	**new_exported;

	new_exported = ft_realloc(*exported, len + 2, sizeof(char *));
	if (!new_exported)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		ft_free_matrix(*exported, len);
		return (1);
	}
	new_exported[len] = ft_strdup(arg);
	if (!new_exported[len])
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		ft_free_matrix(*exported, len + 1);
		return (1);
	}
	new_exported[len + 1] = NULL;
	*exported = new_exported;
	return (0);
}

size_t	add_variables(char **args, char ***exported, size_t i, size_t *len)
{
	char	*ptr;
	size_t	i;
	int		status;

	
	while (args[i])
	{
		ptr = ft_strchr(args[i], '=');
		if (!ptr)
			return (i);
		status = lookup_var(*exported, args[i], ptr, len);
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

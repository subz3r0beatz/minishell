#include "minishell.h"

static int	unset_env_var(char ***exported, char *unset)
{
	char	**new_exported;
	size_t	i;

	i = 0;
	while ((*exported)[i])
	{
		if (ft_strncmp((*exported)[i], unset, ft_strlen(unset)) == 0)
		{
			free((*exported)[i]);
			ft_mem_shift(&(*exported)[i], 1, sizeof(char *), -1);
			new_exported = ft_realloc(*exported,
					ft_memlen(*exported, sizeof(char *)), sizeof(char *));
			if (!new_exported)
			{
				ft_putstr_fd("minishell: env: malloc: "
					"cannot allocate memory\n", STDERR_FILENO);
				ft_free_matrix(*exported, ft_memlen(*exported, sizeof(char *)));
				return (1);
			}
			*exported = new_exported;
			return (0);
		}
		i++;
	}
	return (0);
}

int	handle_unset(char **args, char ***exported, size_t *i, size_t *j)
{
	char	*unset;

	if (!args[*i][*j + 1] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'u'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (2);
	}
	if (args[*i][*j + 1])
		unset = args[*i][*j + 1];
	else
		unset = args[++*i];
	if (!*unset || ft_strchr(unset, '='))
	{
		ft_putstr_fd("minishell: env: cannot unset '", STDERR_FILENO);
		ft_putstr_fd(unset, STDERR_FILENO);
		ft_putstr_fd("': Invalid argument\n", STDERR_FILENO);
		return (2);
	}
	if (unset_env_var(exported, unset))
		return (1);
	*j = ft_strlen(args[*i]) - 1;
	return (0);
}

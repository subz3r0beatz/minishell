#include "minishell.h"


static char	**add_new_args(char	**args, char *string, size_t i)
{
	char	**split;
	char	**new_args;
	size_t	split_size;
	size_t	args_size;

	split = ft_split_quotes(string, " ");
	if (!split)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	split_size = ft_memlen(split, sizeof(char *));
	args_size = ft_memlen(args, sizeof(char *));
	new_args = ft_realloc(args, split_size + args_size + 1, sizeof(char *));
	if (!new_args)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	ft_memmove(&new_args[args_size], &new_args[i + 1],
		(args_size - i - 1) * sizeof(char *));
	ft_memmove(&new_args[i + 1], split, split_size * sizeof(char *));
	return (new_args);
}

int	handle_string(char **args, size_t *i, size_t *j)
{
	char	*string;
	char	**new_args;

	if (!args[*i][*j + 1] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'S'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (2);
	}
	if (args[*i][*j + 1])
		string = args[*i][*j + 1];
	else
		string = args[++*i];
	new_args = add_new_args(args, string);
	if (!new_args)
		return (2);
	args = new_args;
	*j = ft_strlen(args[*i]) - 1;
	return (0);
}

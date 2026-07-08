#include "minishell.h"

int	parse_chdir_path(char **args, t_flags *flags, size_t *i, size_t *j)
{
	if (!args[*i][*j + 1] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'C'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (2);
	}
	if (args[*i][*j + 1])
		flags->chdir_path = ft_strdup(&args[*i][*j + 1]);
	else
		flags->chdir_path = ft_strdup(args[++*i]);
	if (!flags->chdir_path)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (2);
	}
	*j = ft_strlen(args[*i]) - 1;
	return (0);
}

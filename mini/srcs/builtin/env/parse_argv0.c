#include "minishell.h"

int	parse_argv0(char **args, t_flags *flags, size_t *i, size_t *j)
{
	if (!args[*i][*j + 1] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'a'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (2);
	}
	if (args[*i][*j + 1])
		flags->custom_argv0 = ft_strdup(&args[*i][*j + 1]);
	else
		flags->custom_argv0 = ft_strdup(args[++*i]);
	if (!flags->custom_argv0)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (2);
	}
	*j = ft_strlen(args[*i]) - 1;
	return (0);
}

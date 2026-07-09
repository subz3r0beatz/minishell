#include "minishell.h"

int	parse_argv0(char **args, t_flags *flags, size_t *i, size_t *j)
{
	*j++;
	if (!args[*i][*j] && !args[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'a'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if (args[*i][*j])
		flags->custom_argv0 = ft_strdup(&args[*i][*j]);
	else
		flags->custom_argv0 = ft_strdup(args[++*i]);
	if (!flags->custom_argv0)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (1);
	}
	*j = ft_strlen(args[*i]) - 1;
	return (0);
}

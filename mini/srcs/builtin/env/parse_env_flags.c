#include "minishell.h"
#include <stddef.h>

static t_flags	init_flags(void)
{
	t_flags	*flags;

	flags = malloc(sizeof(flags));
	if (!flags)
	{
		ft_putstr_fd("minishell: env: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	flags->ignore_env = 0;
	flags->null_term = 0;
	flags->chdir_path = NULL;
	flags->custom_argv0 = NULL;
	return (flags);
}

static	int	check_flags(char	**args, t_flags *flags)
{
	size_t	i;

	while	(args[0][i])
	{
		if (args[0][i] == '-')
	}
}

size_t	parse_env_flags(char **args, t_flags *flags)
{
	size_t	i;

	flags = init_flags();
	if (!flags)
		return (0);
	i = 1;
	while (args[i] && args[i][0] == '-')
	{
		if (args[i][1] == '-' && !args[i][2])
			return (i + 1);
		if (check_flags(&args[i], flags))
			return (0);
		i++;
	}
	return (i);
}

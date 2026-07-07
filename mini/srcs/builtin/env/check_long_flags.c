/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_long_flags.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 18:15:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 18:21:14 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	long_flag_error(char *arg)
{
	ft_putstr_fd("minishell: env: invalid option '", STDERR_FILENO);
	ft_putstr_fd(arg, STDERR_FILENO);
	ft_putstr_fd("'\nTry 'env --help' for more information.\n",
		STDERR_FILENO);
	return (1);
}

static int	check_long_flags(char **args, t_flags *flags)
{
	char	*ptr;
	size_t	i;

	i = 1;
	while (args[0][++i])
	{
		ptr = ft_strnstr(&args[0][i], "ignore-environment", 18);
		if (ptr && ptr[18] == '\0')
			flags->ignore_env = 1;
		ptr = ft_strnstr(&args[0][i], "null", 4);
		if (ptr && ptr[4] == '\0')
			flags->null_term = 1;
		ptr = ft_strnstr(&args[0][i], "argv0", 5);
		if (ptr && ptr[5] == '\0')
			flags->custom_argv0 = args[0][i + 1];
		ptr = ft_strnstr(&args[0][i], "chdir", 5);
		if (ptr && ptr[5] == '\0')
			flags->chdir_path = args[0][i + 1];
		ptr = ft_strnstr(&args[0][i], "help", 4);
		if (ptr && ptr[4] == '\0')
			flags->print_help = 1;
		ptr = ft_strnstr(&args[0][i], "unset", 5);
		if (ptr && ptr[5] == '\0')
			handle_unset(&args[0][i + 1], environment);
		if (!ptr)
			return (long_flag_error(args[0]));
	}
	return (0);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_env.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 03:45:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 16:56:10 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	print_env(void *key, void *value, void *args)
{
	int		null;
	int		fd_out;
	t_env	*env;

	fd_out = *(int *)args[0];
	null = *(int *)args[1];
	env = (t_env *)value;
	if (env->is_exported && env->value)
	{
		ft_putstr_fd((char *)key, fd_out);
		ft_putchar_fd('=', fd_out);
		ft_putstr_fd(env->value, fd_out);
		if (null)
			ft_putchar_fd('\0', fd_out);
		else
			ft_putchar_fd('\n', fd_out);
	}
}

static int parse_flags(char **args)
{
  size_t  i;
  size_t  j;

  i = 1;
  while (args[i] && args[i][0] == '-')
  {
    if (args[i][1] == '-' && !args[i][2])
      return (i + 1);
    i++;
  }
}

int	ft_env(t_minishell minishell, char **args, int fd_out)
{
  size_t  i;
  int     print;
  char    **exported;

  exported = env_to_matrix(minishell->env);
  i = parse_flags(args, exported);
}

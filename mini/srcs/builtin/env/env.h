/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 16:57:02 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 06:58:25 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENV_H
# define ENV_H

# include "parse_env/parse_env.h"
# include "exec_env/exec_env.h"

typedef struct s_flags
{
	int		fd_out;
	int		print_help;
	int		ignore_env;
	int		null_term;
	char	*chdir_path;
	char	*custom_argv0;
}				t_flags;

typedef struct s_max_uints
{
	size_t	i;
	size_t	j;
	size_t	capacity;
	size_t	exported_len;
	size_t	args_len;
}				t_max_uints;

int		ft_env(t_minishell *shell, char **args, int fd_out);
int		exit_env(char **matrices[2], t_flags *flags,
			t_max_uints *max_uints, int exit_code);
void	print_env(char **exported, int null, int fd_out);

#endif

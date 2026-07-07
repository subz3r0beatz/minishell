/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 16:57:02 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 16:57:51 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENV_H
# define ENV_H

typedef struct s_flags
{
	int		ignore_env;
	int		null_term;
	char	*chdir_path;
	char	*custom_argv0;
}								t_flags;

int			ft_env(t_minishell *shell, char **argv, int fd_out);
size_t	parse_env_flags(char **args, t_flags *flags);

#endif

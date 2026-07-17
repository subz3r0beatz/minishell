/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 06:53:16 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/17 02:56:40 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILTIN_H
# define BUILTIN_H

# include "export/export.h"
# include "env/env.h"
# include "cd/cd.h"

typedef struct s_minishell	t_minishell;

int		ft_echo(t_minishell *shell, char **args, int fd_out);
int		ft_pwd(t_minishell *shell, char **args, int fd_out);
int		ft_unset(t_minishell *shell, char **args, int fd_out);

#endif

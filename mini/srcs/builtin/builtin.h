/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 06:53:16 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 17:25:49 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILTIN_H
# define BUILTIN_H

# include "export.h"
# include "env.h"

int		ft_echo(char **args, int fd_out);
int		ft_pwd(t_robin *env, char **args, int fd_out);
int		ft_unset(t_robin *env, char **args);

#endif

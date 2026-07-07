/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.h                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 13:37:42 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/02 13:38:21 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CD_H
# define CD_H

int		ft_cd(t_minishell *shell, char **args, int fd_out);
char	*canonalize_path(char *pwd, char *path);
int		parse_dir(t_minishell *shell, char *arg, char **dir, int *print_path);
int		move_dir(t_minishell *shell, char **dir, int logical, int e_flag);

#endif

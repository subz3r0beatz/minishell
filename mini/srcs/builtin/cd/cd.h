/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.h                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 13:37:42 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 13:22:16 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CD_H
# define CD_H

int		ft_cd(t_minishell *shell, char **args);
char	*canonalize_path(t_minishell *shell, char *pwd, char *path);
int		parse_dir(t_minishell *shell, char *arg, char **dir, int *old_or_home);
int		move_dir(t_minishell *shell, char **dir, int logical, int e_flag);
int		update_vars(t_minishell *shell, char *dir);

#endif

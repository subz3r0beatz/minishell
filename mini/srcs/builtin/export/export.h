/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 15:14:22 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/17 02:54:41 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXPORT_H
# define EXPORT_H

int	print_export(t_robin *env, int fd_out);
int	parse_export_vars(t_minishell *shell, char **args);
int	unexport_vars(t_minishell *shell, char **args);
int	ft_export(t_minishell *shell, char **args, int fd_out);

#endif

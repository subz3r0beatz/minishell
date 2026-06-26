/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 15:14:22 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 15:55:26 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXPORT_H
# define EXPORT_H

int	print_export(t_robin *env, int fd_out);
int	parse_vars(t_robin *env, char **args);
int	ft_export(t_robin *env, char **args, int fd_out);
int	unexport_vars(t_robin *env, char **args);

#endif

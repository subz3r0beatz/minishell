/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_env.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 12:03:09 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/13 20:51:49 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXEC_ENV_H
# define EXEC_ENV_H

int		exec_env(char **args, char **exported, t_flags *flags, size_t lens[2]);
char	*resolve_cmd_path(char **args, char **exported);

#endif

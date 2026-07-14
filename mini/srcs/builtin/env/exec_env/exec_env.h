/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_env.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 12:03:09 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 17:30:05 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXEC_ENV_H
# define EXEC_ENV_H

int		exec_env(char **matrices[2], t_flags *flags, t_max_uints *max_uints);
char	*resolve_cmd_path(char **args, char **exported,
			int *exit_code, int *no_malloc_error);

#endif

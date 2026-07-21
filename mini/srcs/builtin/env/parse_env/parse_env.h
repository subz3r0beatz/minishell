/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_env.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 12:15:58 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/21 23:54:01 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSE_ENV_H
# define PARSE_ENV_H

typedef struct s_max_uints	t_max_uints;
typedef struct s_flags		t_flags;

size_t	add_variables(char **matrices[2], t_max_uints *max_uints);
int		handle_long_split(char ***args, size_t *i, size_t *j,
			size_t *args_size);
int		handle_long_unset(char **matrices[2], t_max_uints *max_uints);
int		handle_split(char ***args, size_t *i, size_t *j, size_t *args_size);
int		handle_unset(char **matrices[2], t_max_uints *max_uints);
int		parse_argv0(char **args, t_flags *flags, size_t *i, size_t *j);
int		parse_chdir_path(char **args, t_flags *flags, size_t *i, size_t *j);
size_t	parse_env_flags(char **matrices[2],
			t_flags *flags, t_max_uints *max_uints);
int		parse_long_argv0(char **args, t_flags *flags, size_t *i, size_t *j);
int		parse_long_chdir_path(char **args, t_flags *flags,
			size_t *i, size_t *j);
int		parse_long_flags(char **matrices[2],
			t_flags *flags, t_max_uints *max_uints);
int		parse_short_flags(char **matrices[2],
			t_flags *flags, t_max_uints *max_uints);
int		print_env_help(int fd_out);

#endif

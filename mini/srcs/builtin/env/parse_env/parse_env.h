/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_env.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 12:15:58 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/13 19:58:23 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSE_ENV_H
# define PARSE_ENV_H

int		check_flags(char ***args, char ***exported,
			t_flags *flags, size_t max_uints[4]);
int		check_long_flags(char ***args, char ***exported,
			t_flags *flags, size_t max_uints[4]);
int		handle_split(char ***args, size_t *i, size_t *j, size_t *args_size);
int		handle_unset(char **args, char ***exported, size_t max_uints[4]);
int		parse_argv0(char **args, t_flags *flags, size_t *i, size_t *j);
int		parse_chdir_path(char **args, t_flags *flags, size_t *i, size_t *j);
int		handle_long_split(char ***args, size_t *i, size_t *j,
			size_t *args_size);
int		handle_long_unset(char **args, char ***exported, size_t max_uints[4]);
int		parse_long_argv0(char **args, t_flags *flags, size_t *i, size_t *j);
int		parse_long_chdir_path(char **args, t_flags *flags,
			size_t *i, size_t *j);
int		print_env_help(int fd_out);
size_t	parse_env_flags(char ***args, char ***exported,
			t_flags *flags, size_t lens[2]);
size_t	add_variables(char **args, char ***exported,
			size_t max_uints[2], size_t *capacity);
#endif

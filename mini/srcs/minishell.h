/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 21:35:52 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/19 18:43:25 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

// Bright Text Colors
# define C_B_BLACK   "\001\033[1;30m\002"
# define C_B_RED     "\001\033[1;31m\002"
# define C_B_GREEN   "\001\033[1;32m\002"
# define C_B_YELLOW  "\001\033[1;33m\002"
# define C_B_BLUE    "\001\033[1;34m\002"
# define C_B_MAGENTA "\001\033[1;35m\002"
# define C_B_CYAN    "\001\033[1;36m\002"
# define C_B_WHITE   "\001\033[1;37m\002"

// Standard Text Colors
# define C_BLACK   "\001\033[0;30m\002"
# define C_RED     "\001\033[0;31m\002"
# define C_GREEN   "\001\033[0;32m\002"
# define C_YELLOW  "\001\033[0;33m\002"
# define C_BLUE    "\001\033[0;34m\002"
# define C_MAGENTA "\001\033[0;35m\002"
# define C_CYAN    "\001\033[0;36m\002"
# define C_WHITE   "\001\033[0;37m\002"

// Formatting and Resets
# define C_RESET     "\001\033[0m\002"
# define C_BOLD      "\001\033[1m\002"
# define C_UNDERLINE "\001\033[4m\002"
# define C_REVERSED  "\001\033[7m\002" // Swaps foreground and background colors

# include "includes.h"
# include "../libft/libft.h"
# include "robin_hash/robin_hash.h"
# include "environment/environment.h"
# include "builtin/builtin.h"
# include "prompt/prompt.h"
# include "lookup_table/lookup_table.h"
//# include "error/error.h"

typedef struct s_minishell
{
	t_robin			*env;
	char			**exported;
	size_t			exported_count;
	uint8_t			token_type_table[256][256];
}				t_minishell;

#endif

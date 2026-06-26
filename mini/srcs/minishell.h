/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 21:35:52 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 19:33:05 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "includes.h"
# include "libft.h"
# include "robin_hash.h"
# include "environment.h"
# include "builtin.h"
# include "parser.h"
# include "prompt.h"
# include "lexer.h"
# include "lookup_table.h"
# include "error.h"

typedef struct s_minishell
{
	t_robin	*env;
	char	**exported;
	uint8_t	token_type_table[256][256];
}				t_minishell;

#endif

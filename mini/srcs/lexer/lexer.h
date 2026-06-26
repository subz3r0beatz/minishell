/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 09:16:07 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 14:11:03 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_H
# define LEXER_H

typedef enum e_token_type
{
	TOKEN_WORD,
	TOKEN_OR,
	TOKEN_AND,
	TOKEN_DLESS,
	TOKEN_DGREAT,
	TOKEN_PIPE,
	TOKEN_BACKGR,
	TOKEN_LESS,
	TOKEN_GREAT,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_SEMI
}				t_token_type;

typedef struct s_token
{
	char			*value;
	t_token_type	type;
	struct s_token	*next;
}				t_token;

t_token	*lexer(char *input, uint8_t table[256][256]);

#endif

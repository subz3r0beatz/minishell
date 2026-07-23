/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_token_type_table.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 16:22:38 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/17 02:46:55 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	init_token_type_table(uint8_t table[256][256])
{
	int	i;
	int	j;

	i = -1;
	while (++i < 256)
	{
		j = -1;
		while (++j < 256)
			table[i][j] = TOKEN_WORD;
	}
	j = -1;
	while (++j < 256)
	{
		table['|'][j] = TOKEN_PIPE;
		table['&'][j] = TOKEN_BACKGR;
		table['<'][j] = TOKEN_LESS;
		table['>'][j] = TOKEN_GREAT;
		table['('][j] = TOKEN_LPAREN;
		table[')'][j] = TOKEN_RPAREN;
		table[';'][j] = TOKEN_SEMI;
	}
	table['|']['|'] = TOKEN_OR;
	table['&']['&'] = TOKEN_AND;
	table['<']['<'] = TOKEN_DLESS;
	table['>']['>'] = TOKEN_DGREAT;
}

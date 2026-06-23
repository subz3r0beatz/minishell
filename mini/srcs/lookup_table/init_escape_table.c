/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_escape_table.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 16:25:20 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/23 16:30:04 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	init_escape_table(char table[256])
{
	unsigned char	i;

	i = 0;
	while (i < 256)
	{
		table[i] = 0;
		i++;
	}
	table['\\'] = '\\';
	table['a'] = '\a';
	table['b'] = '\b';
	table['e'] = 27;
	table['f'] = '\f';
	table['n'] = '\n';
	table['r'] = '\r';
	table['t'] = '\t';
	table['v'] = '\v';
}

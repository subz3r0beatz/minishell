/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_word_token_len.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 18:38:24 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/18 18:38:47 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

size_t	get_word_token_len(char *input, uint8_t table[256][256])
{
	size_t	len;
	char	quote_state;

	len = 0;
	quote_state = 0;
	while (input[len])
	{
		if (quote_state != '\'' && input[len] == '\\' && input[len + 1])
		{
			len += 2;
			continue ;
		}
		if (quote_state == 0 && (input[len] == '"' || input[len] == '\''))
			quote_state = input[len];
		else if (quote_state == input[len])
			quote_state = 0;
		if (quote_state == 0 && ((ft_iswhite(input[len]) && input[len] != '\n')
				|| ((t_token_type)table[(unsigned char)input[len]]
					[(unsigned char)input[len + 1]] != TOKEN_WORD
					&& (t_token_type)table[(unsigned char)input[len]]
					[(unsigned char)input[len + 1]] != TOKEN_COMMENT)))
			break ;
		len++;
	}
	return (len);
}

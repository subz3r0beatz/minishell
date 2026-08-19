/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_unclosed_quotes.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 19:59:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/18 21:03:19 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	check_unclosed_quotes(const char *input, char *quote_state)
{
	size_t	i;
	char	quote;

	i = 0;
	quote = 0;
	while (input && input[i])
	{
		if ((quote == 0 && input[i] == '\\' && input[i + 1])
			|| (quote == '"' && input[i] == '\\'
				&& input[i + 1] && ft_strchr("\\\n`$\"'", input[i + 1])))
			i++;
		else if (quote == 0 && (input[i] == '"' || input[i] == '\''))
			quote = input[i];
		else if (quote != 0 && input[i] == quote)
			quote = 0;
		i++;
	}
	if (quote_state)
		*quote_state = quote;
	return (quote != 0);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extract_line.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 20:26:17 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/17 23:45:37 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	loop_to_next_line(char *start, int quote)
{
	size_t	i;
	char	quote_state;

	i = 0;
	quote_state = 0;
	while (start[i])
	{
		if (quote && !quote_state && (start[i] == '"' || start[i] == '\''))
			quote_state = start[i];
		else if (quote && quote_state && start[i] == quote_state)
			quote_state = 0;
		else if (quote && !quote_state && start[i] == '\n')
		{
			if (i > 0 && start[i - 1] == '\\')
			{
				i++;
				continue ;
			}
			break ;
		}
		if (!quote && start[i] == '\n')
			break ;
		i++;
	}
	return (i);
}

char	*extract_line(char	**ptr, int quote, int *malloc_error)
{
	char	*start;
	char	*line;
	size_t	i;

	*malloc_error = 0;
	start = *ptr;
	if (!start || !*start)
		return (NULL);
	i = loop_to_next_line(start, quote);
	line = ft_substr(start, 0, i);
	if (!line)
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		*malloc_error = 1;
		return (NULL);
	}
	if (start[i] == '\n')
		*ptr = start + i + 1;
	else
		*ptr = start + i;
	return (line);
}

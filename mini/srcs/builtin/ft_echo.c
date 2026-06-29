/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_echo.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 21:31:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 15:14:42 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	parse_flags(char **args, int *newline, int *escape)
{
	size_t	i;
	size_t	j;

	i = 1;
	while (args[i] && args[i][0] == '-' && args[i][1] != '\0')
	{
		j = 1;
		while (args[i][j])
		{
			if (!ft_strchr("neE", args[i][j]))
				return (i);
			if (args[i][j] == 'n')
				*newline = 0;
			else (args[i][j] == 'e')
				*escape = 1;
			if (args[i][j] == 'E')
				*escape = 0;
			j++;
		}
		i++;
	}
	return (i + 1);
}

static char	get_hex(char *str, size_t *i)
{
	int		val;
	int		count;
	char	c;

	*i += 2;
	val = 0;
	count = 0;
	while (count < 2 && str[*i])
	{
		c = ft_tolower(str[*i]);
		if (c >= '0' && c <= '9')
			val = val * 16 + (c - '0');
		else if (c >= 'a' && c <= 'f')
			val = val * 16 + (c - 'a' + 10);
		else
			break ;
		(*i)++;
		count++;
	}
	return ((char)val);
}

static char	get_char(char *str, size_t *i, char table[256])
{
	int		val;
	char	c;
	char	*ptr;

	if (str[*i] != '\\' || !str[*i + 1])
		return (str[(*i)++]);
	if (str[*i + 1] == '0')
	{
		*i += 2;
		val = 0;
		c = -1;
		while (++c < 3 && str[*i] >= '0' && str[*i] <= '7')
			val = val * 8 + (str[(*i)++] - '0');
		return ((char)val);
	}
	c = str[*i + 2];
	if (str[*i + 1] == 'x' && c && ((c >= '0' && c <= '9')
			|| (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
		return (get_hex(str, i));
	ptr = ft_strchr("abefnrtv\\", ft_tolower(str[*i + 1]));
	if (!ptr)
		return (str[(*i)++]);
	*i += 2;
	return (table[ft_tolower(str[*i - 1])]);
}

static int	print_escape(char *str, int fd_out, char table[256])
{
	size_t	i;
	size_t	j;
	char		*buffer;

	buffer = malloc(sizeof(char) * (ft_strlen(str) + 1));
	if (!buffer)
		return (1);
	i = 0;
	j = 0;
	while (str[i])
	{
		if (str[i] == '\\' && str[i + 1] == 'c')
		{
			write(fd_out, buffer, j);
			free(buffer);
			return (1);
		}
		buffer[j++] = get_char(str, &i, table);
	}
	write(fd_out, buffer, j);
	free(buffer);
	return (0);
}

int	ft_echo(t_minishell *shell, char **args, int fd_out)
{
	size_t	i;
	int		newline;
	int		escape;
	char	table[256];

	newline = 1;
	escape = 0;
	init_escape_table(table);
	i = parse_flags(args, &newline, &escape) - 1;
	while (args[++i])
	{
		if (escape && print_escape(args[i], fd_out, table))
			return (0);
		if (!escape)
			ft_putstr_fd(args[i], fd_out);
		if (args[i + 1])
			ft_putchar_fd(' ', fd_out);
	}
	if (newline)
		ft_putchar_fd('\n', fd_out);
	return (0);
}

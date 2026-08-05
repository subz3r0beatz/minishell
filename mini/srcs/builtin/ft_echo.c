/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_echo.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 21:31:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 14:14:49 by fldumas-         ###   ########.fr       */
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
			if (args[i][j] == 'n')
				*newline = 0;
			else if (args[i][j] == 'e')
				*escape = 1;
			else if (args[i][j] == 'E')
				*escape = 0;
			else
				return (i);
			j++;
		}
		i++;
	}
	return (i);
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

static int	print_escape(char *str, char table[256])
{
	size_t	i;
	size_t	j;
	char	*buffer;

	buffer = malloc(sizeof(char) * (ft_strlen(str) + 1));
	if (!buffer)
		ft_putstr_fd("minishell: echo: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	if (!buffer)
		return (1);
	i = 0;
	j = 0;
	while (str[i])
	{
		if (str[i] == '\\' && str[i + 1] == 'c')
		{
			write(STDOUT_FILENO, buffer, j);
			free(buffer);
			return (1);
		}
		buffer[j++] = get_char(str, &i, table);
	}
	write(STDOUT_FILENO, buffer, j);
	free(buffer);
	return (0);
}

int	ft_echo(t_minishell *shell, char **args)
{
	size_t	i;
	int		newline;
	int		escape;
	char	table[256];

	(void)shell;
	newline = 1;
	escape = 0;
	init_escape_table(table);
	i = parse_flags(args, &newline, &escape) - 1;
	while (args[++i])
	{
		if (escape && print_escape(args[i], table))
			return (0);
		if (!escape)
			ft_putstr_fd(args[i], STDOUT_FILENO);
		if (args[i + 1])
			ft_putchar_fd(' ', STDOUT_FILENO);
	}
	if (newline)
		ft_putchar_fd('\n', STDOUT_FILENO);
	return (0);
}

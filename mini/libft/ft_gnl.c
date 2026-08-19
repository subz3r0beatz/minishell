/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_gnl.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 18:16:11 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/17 18:17:04 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	append_buffer(char	**line, char *buffer)
{
	char	*tmp;
	size_t	s1;
	size_t	i;

	s1 = ft_strlen(*line);
	i = 0;
	while (buffer[i] && buffer[i] != '\n')
		i++;
	if (buffer[i] == '\n')
		i++;
	tmp = malloc(s1 + i + 1);
	if (!tmp)
	{
		free(*line);
		*line = NULL;
		return (1);
	}
	ft_memcpy(tmp, *line, s1);
	ft_memcpy(tmp + s1, buffer, i);
	tmp[s1 + i] = '\0';
	free(*line);
	*line = tmp;
	ft_memmove(buffer, buffer + i, ft_strlen(buffer + i) + 1);
	return (tmp[s1 + i - 1] == '\n');
}

char	*ft_gnl(int fd, char *buffer, size_t buffer_size, int *malloc_error)
{
	char	*line;
	ssize_t	bytes;

	if (malloc_error)
		*malloc_error = 1;
	line = NULL;
	bytes = -1;
	if (fd >= 0 && buffer && buffer_size >= 2)
	{
		while (1)
		{
			if (*buffer && append_buffer(&line, buffer))
				return (line);
			bytes = read(fd, buffer, buffer_size - 1);
			if (bytes <= 0)
				break ;
			buffer[bytes] = '\0';
		}
	}
	if (bytes == 0 && line && *line)
		return (line);
	if (malloc_error)
		*malloc_error = 0;
	free(line);
	return (NULL);
}

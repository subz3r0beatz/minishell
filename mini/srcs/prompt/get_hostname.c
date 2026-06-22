/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_hostname.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 01:05:43 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/12 06:39:33 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	read_fd(char *buffer)
{
	int	fd;
	int	bytes_read;

	fd = open("/etc/hostname", O_RDONLY);
	if (fd < 0)
		return (-1);
	bytes_read = read(fd, buffer, 8191);
	close(fd);
	return (bytes_read);
}

char	*get_hostname(t_robin *env, char *buffer)
{
	int				i;
	int				bytes_read;
	char			*hostname;
	t_robin_node	*node;

	node = robin_search(env, "HOSTNAME");
	if (node && node->value && ((t_env *)node->value)->value)
		return (ft_strdup(((t_env *)node->value)->value));
	bytes_read = read_fd(buffer);
	if (bytes_read <= 0)
		return (NULL);
	buffer[bytes_read] = '\0';
	i = 0;
	while (buffer[i] && buffer[i] != '\n' && buffer[i] != '.')
		i++;
	hostname = (char *)malloc(sizeof(char) * (i + 1));
	if (!hostname)
		return (NULL);
	hostname[i] = '\0';
	while (--i >= 0)
		hostname[i] = buffer[i];
	return (hostname);
}

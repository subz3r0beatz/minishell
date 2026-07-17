/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_username.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 01:01:10 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 22:34:30 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	read_file(char *path, char *buffer, int size)
{
	int	fd;
	int	bytes_read;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (-1);
	bytes_read = read(fd, buffer, size - 1);
	close(fd);
	if (bytes_read > 0)
		buffer[bytes_read] = '\0';
	return (bytes_read);
}

static char	*search_passwd_username(uid_t uid, char *buffer)
{
	char	*start;
	uid_t	id;
	int		colons;

	while (*buffer)
	{
		start = buffer;
		colons = 0;
		id = 0;
		while (*buffer && *buffer != '\n')
		{
			if (*buffer == ':' && colons++ <= 1)
				*buffer = '\0';
			else if (colons == 2 && ft_isdigit(*buffer))
				id = id * 10 + (*buffer - '0');
			buffer++;
		}
		if (colons >= 2 && id == uid)
			return (ft_strdup(start));
		if (*buffer == '\n')
			buffer++;
	}
	return (NULL);
}

char	*get_username(t_robin *env, char *buffer)
{
	struct stat		st;
	char			*usr;
	t_robin_node	*node;

	node = robin_search(env, "USER");
	if (node && node->value && ((t_env *)node->value)->value)
		return (ft_strdup(((t_env *)node->value)->value));
	node = robin_search(env, "LOGNAME");
	if (node && node->value && ((t_env *)node->value)->value)
		return (ft_strdup(((t_env *)node->value)->value));
	if (stat("/proc/self", &st) < 0)
	{
		usr = ttyname(0);
		if (!usr || stat(usr, &st) < 0)
			return (NULL);
	}
	if (read_file("/etc/passwd", buffer, 8192) > 0)
	{
		usr = search_passwd_username(st.st_uid, buffer);
		if (usr)
			return (usr);
	}
	return (NULL);
}

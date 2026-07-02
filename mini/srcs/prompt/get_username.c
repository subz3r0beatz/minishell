/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_username.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 01:01:10 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/02 14:21:43 by fldumas-         ###   ########.fr       */
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

static char	*search_passwd(uid_t uid, char *buffer)
{
	char	*ptr;
	char	*start;
	uid_t	id;
	int		colons;

	ptr = buffer;
	while (*ptr)
	{
		start = ptr;
		colons = 0;
		id = 0;
		while (*ptr && *ptr != '\n')
		{
			if (*ptr == ':' && colons++ <= 1)
				*ptr = '\0';
			else if (colons == 2)
				id = id * 10 + *ptr - '0';
			ptr++;
		}
		if (colons >= 2 && id == uid)
			return (ft_strdup(start));
		if (*ptr == '\n')
			ptr++;
	}
	return (NULL);
}

static char	*extract_username(char *pid, uid_t uid, char *buffer)
{
	struct stat	st;
	char		path[MAX_PATH];
	int			bytes_read;
	int			i;

	ft_strlcpy(path, "/proc/", sizeof(path));
	ft_strlcat(path, pid, sizeof(path));
	ft_strlcat(path, "/environ", sizeof(path));
	if (stat(path, &st) < 0 || st.st_uid != uid)
		return (NULL);
	bytes_read = read_file(path, buffer, sizeof(buffer));
	i = 0;
	while (i < bytes_read)
	{
		if (ft_strncmp(&buffer[i], "USER=", 5) == 0)
			return (ft_strdup(&buffer[i + 5]));
		while (i < bytes_read && buffer[i] != '\0')
			i++;
		i++;
	}
	return (NULL);
}

static char	*search_proc(uid_t uid, char *buffer)
{
	struct dirent	*entry;
	DIR				*dir;
	char			*usr;

	dir = opendir("/proc");
	if (!dir)
		return (NULL);
	usr = NULL;
	entry = readdir(dir);
	while (entry)
	{
		if (ft_isdigit(entry->d_name[0]))
		{
			usr = extract_username(entry->d_name, uid, buffer);
			if (usr)
				break ;
		}
		entry = readdir(dir);
	}
	closedir(dir);
	return (usr);
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
		usr = search_passwd(st.st_uid, buffer);
		if (usr)
			return (usr);
	}
	return (search_proc(st.st_uid, buffer));
}

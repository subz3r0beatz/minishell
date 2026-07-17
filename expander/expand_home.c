/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_home.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 22:35:26 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/16 23:50:39 by fldumas-         ###   ########.fr       */
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

static char	*search_passwd_home(uid_t uid, char *buffer)
{
	char	*start;
	uid_t	id;
	int		colons;

	while (*buffer)
	{
		start = NULL;
		colons = 0;
		id = 0;
		while (*buffer && *buffer != '\n')
		{
			if (*buffer == ':' && colons == 4)
				start = buffer + 1;
			if (*buffer == ':' && colons++ <= 5)
				*buffer = '\0';
			else if (colons == 2 && ft_isdigit(*ptr))
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

static char	*get_home(void)
{
	struct stat	st;
	char		*home;
	char		buffer[MAX_PATH];

	if (stat("/proc/self", &st) < 0)
	{
		home = ttyname(0);
		if (!home || stat(home, &st) < 0)
			return (NULL);
	}
	if (read_file("/etc/passwd", buffer, MAX_PATH) > 0)
	{
		home = search_passwd_home(st.st_uid, buffer);
		if (home)
			return (home);
	}
	return (NULL);
}

char	*expand_home(t_minishell *shell, char *word)
{
	char	*home;

	free(word);
	if (get_var_value(shell, "HOME", &home) || !home)
		home = get_home();
	else
		home = ft_strdup(home);
	if (!home)
		return (NULL);
	return (home);
}

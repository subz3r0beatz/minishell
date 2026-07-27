/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_pid.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 14:18:58 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/26 16:30:57 by fldumas-         ###   ########.fr       */
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

int	handle_pid(t_minishell *shell)
{
	char	buffer[16];
	size_t	i;

	if (read_file("/proc/self/stat", buffer, 16) <= 0)
	{
		shell->pid = ft_strdup("");
		if (!shell->pid)
			return (1);
		return (0);
	}
	i = 0;
	while (buffer[i] && buffer[i] != ' ')
		i++;
	shell->pid = ft_substr(buffer, 0, i);
	if (!shell->pid)
		return (1);
	return (0);
}

#include "minishell.h"

ssize_t	open_read(char *path, char *buffer, size_t size)
{
	int	fd;
	ssize_t	bytes_read;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (-1);
	bytes_read = read(fd, buffer, size);
	close(fd);
	return (bytes_read);
}

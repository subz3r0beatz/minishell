#include "minishell.h"

static char	*match_uid(uid_t uid, char *line,
	size_t sec, int *malloc_error)
{
	uid_t		id;
	size_t	col;
	char	*start;

	id = 0;
	col = 0;
	start = line;
	while (*line && (col <= sec || col <= 2) && (col <= 2 || id == uid))
	{
		if (*line == ':' || (*line == '\n' && !*(line + 1)))
		{
			col++;
			*line = 0;
			if (col == sec)
				start = line + 1;
		}
		else if (col == 2 && ft_isdigit(*line))
			id = id * 10 + (*line - '0');
		line++;
	}
	if (col >= 2 && col >= sec && id == uid)
		return (ft_strdup(start));
	*malloc_error = 0;
	return (NULL);
}

static char *search_uid(int fd, struct stat st, size_t section, int *malloc_error)
{
	char	*line;
	char	*match;
	char	buffer[128];

	buffer[0] = 0;
	while (1)
	{
		*malloc_error = 1;
		line = ft_gnl(fd, buffer, 128, malloc_error);
		if (!line)
		{
			close(fd);
			return (NULL);
		}
		*malloc_error = 1;
		match = match_uid(st.st_uid, line, section, malloc_error);
		free(line);
		if (match || *malloc_error == 1)
		{
			close(fd);
			return (match);
		}
	}
}

char	*search_passwd(size_t section, int *malloc_error)
{
	int				fd;
	struct stat	st;
	char		*tty;

	*malloc_error = 0;
	if (stat("/proc/self", &st) < 0)
	{
		tty = ttyname(0);
		if (!tty || stat(tty, &st) < 0)
			return (NULL);
	}
	fd = open("/etc/passwd", O_RDONLY);
	if (fd < 0)
		return (NULL);
	return (search_uid(fd, st, section, malloc_error));
}

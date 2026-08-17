#include "minishell.h"

int	handle_herestring(char *file)
{
	int	pfd[2];

	if (pipe(pfd) < 0)
	{
		ft_putstr_fd("minishell: pipe failed\n", STDERR_FILENO);
		return (-1);
	}
	if (file)
		ft_putstr_fd(file, pfd[1]);
	ft_putstr_fd("\n", pfd[1]);
	close(pfd[1]);
	return (pfd[0]);
}

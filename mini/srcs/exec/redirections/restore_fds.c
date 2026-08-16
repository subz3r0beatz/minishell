#include "minishell.h"

int	restore_fds(int in, int out)
{
	if (in >= 0 && in != STDIN_FILENO)
	{
		if (dup2(in, STDIN_FILENO) < 0)
		{
			perror("minishell: dup2");
			close(in);
			if (out >= 0)
				close(out);
			return (1);
		}
		close(in);
	}
	if (out >= 0 && out != STDOUT_FILENO)
	{
		if (dup2(out, STDOUT_FILENO) < 0)
		{
			perror("minishell: dup2");
			close(out);
			return (1);
		}
		close(out);
	}
	return (0);
}

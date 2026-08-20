#include "minishell.h"

char	*quote_error_handler()
{
	if (!err || (!next && *err))
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	shell->syn_err = 1;
	if (g_signal_status == 130)
	{
		shell->exit_status = 130;
		free(next);
		return (NULL);
	}
	ft_putstr_fd("minishell: unexpected EOF while "
		"looking for matching `", STDERR_FILENO);
	ft_putchar_fd(quote, STDERR_FILENO);
	ft_putstr_fd("'\nminishell: syntax error: "
		"unexpected end of file\n", STDERR_FILENO);
	shell->exit_status = 2;
	return (NULL);
}

char	*heredoc_error_handler(char *next, char *extra, int *err)
{
	if (!err || (!next && *err))
	{
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	if (g_signal_status == 130)
	{
		*malloc_error = 0;
		free(next);
		return (NULL);
	}
	if (!line)
	{
		ft_putstr_fd("minishell: warning: here-document delimited by "
			"end-of-file (wanted `", STDERR_FILENO);
		ft_putstr_fd(extra, STDERR_FILENO);
		ft_putstr_fd("')\n", STDERR_FILENO);
		return (NULL);
	}
	return (line);
}

char	*get_next(t_minishell *shell, char *prompt, char *buffer,
	char *(*error_handler(char *next, char *extra, int *err)))
{
	char	*next;
	char	*tmp;
	int		err;

	if (isatty(STDIN_FILENO))
	{
		rl_event_hook = rl_signal_check
		next = readline(prompt);
		rl_event_hook = NULL;
	}
	else
		next = ft_gnl(STDIN_FILENO, buffer, 128, &err)
	if (!next || g_signal_status == 130)
		return (error_handler(next, extra, &err));
	if (!shell->history || !isatty(STDIN_FILENO))
		return (next);
	tmp = ft_strjoin_3(shell->history, "\n", next);
	if (!tmp)
		return (error_handler(next, extra, NULL));
	free(shell->history);
	shell->history = tmp;
	return (next);
}

int	

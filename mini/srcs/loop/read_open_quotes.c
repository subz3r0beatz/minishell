#include "minishell.h"

static int	handle_quotes(const char *input, char *quote_state)
{
	size_t	i;
	char	quote;

	i = 0;
	quote = 0;
	while (input && input[i])
	{
		if ((quote == 0 && input[i] == '\\' && input[i + 1])
			|| (quote == '"' && input[i] == '\\'
				&& input[i + 1] && ft_strchr("\\\n`$\"'", input[i + 1])))
			i++;
		else if (quote == 0 && (input[i] == '"' || input[i] == '\''))
			quote = input[i];
		else if (quote != 0 && input[i] == quote)
			quote = 0;
		i++;
	}
	*quote_state = quote;
	return (quote != 0);
}

static char	*get_next(char *buffer)
{
	char	*next;
	int		malloc_error;

	malloc_error = 0;
	rl_event_hook = rl_signal_check;
	if (isatty(STDIN_FILENO))
		next = readline("> ");
	else
	{
		malloc_error = 1;
		next = ft_gnl(STDIN_FILENO, buffer, 128, &malloc_error);
	}
	rl_event_hook = NULL;
	if (!next && malloc_error)
		ft_putstr_fd("minishell: malloc: cannot allocate memory\n", STDERR_FILENO);
	return (next);
}

static char	*sigint_sigquit(t_minishell *shell,
	char *input, char *next, char quote)
{
	if (g_signal_status == 130)
	{
		shell->exit_status = 130;
		free(input);
		free(next);
		return (NULL);
	}
	ft_putstr_fd("minishell: unexpected EOF while "
		"looking for matching `", STDERR_FILENO);
	ft_putchar_fd(quote, STDERR_FILENO);
	ft_putstr_fd("'\nminishell: syntax error: "
		"unexpected end of file\n", STDERR_FILENO);
	shell->exit_status = 2;
	free(input);
	return (NULL);
}


char	*read_open_quotes(t_minishell *shell, char *raw_input, char *buffer)
{
	char	*input;
	char	*next;
	char	*tmp;
	char	quote;

	if (!raw_input || !handle_quotes(raw_input, &quote))
		return (raw_input);
	init_interactive_signals(1);
	input = raw_input;
	while (input && handle_quotes(input, &quote))
	{
		g_signal_status = 0;
		next = get_next(buffer);
		if (!next || g_signal_status == 130)
			return (sigint_sigquit(shell, input, next, quote));
		if (isatty(STDIN_FILENO))
			tmp = ft_strjoin_3(input, "\n", next);
		else
			tmp = ft_strjoin(input, next);
		free(input);
		free(next);
		input = tmp;
	}
	if (!input)
		ft_putstr_fd("minishell: malloc: cannot allocate memory\n", STDERR_FILENO);
	return (input);
}

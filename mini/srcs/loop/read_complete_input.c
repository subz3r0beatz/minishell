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

static char	*quote_state_error(t_minishell *shell, char *input, char quote)
{
	ft_putstr_fd("minishell: unexpected EOF while "
		"looking for matching `", STDERR_FILENO);
	ft_putchar_fd(quote, STDERR_FILENO);
	ft_putstr_fd("'\nsyntax error: unexpected end of file\n",
		STDERR_FILENO);
	shell->exit_status = 2;
	free(input);
	return (NULL);
}

char	*read_complete_input(t_minishell *shell, char *raw_input)
{
	char	*input;
	char	*next;
	char	*tmp;
	char	quote;

	input = raw_input;
	while (input && handle_quotes(input, &quote))
	{
		if (isatty(STDIN_FILENO))
			next = readline("> ");
		else
			next = read_line_non_interactive(STDIN_FILENO);
		if (!next)
			return (quote_state_error(shell, input, quote));
		tmp = ft_strjoin_3(input, "\n", next);
		free(input);
		free(next);
		input = tmp;
	}
	if (!input)
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	return (input);
}

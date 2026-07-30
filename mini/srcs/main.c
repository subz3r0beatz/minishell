/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 17:46:29 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/30 01:55:41 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	process_input(t_minishell *shell, char *input, char *argv0)
{
	t_token		*tokens;
	t_ast_node	*ast;

	add_history(input);
	tokens = lexer(input, shell->token_type_table);
	if (!tokens)
		return ;
	ast = parser(tokens);
	if (!ast)
		return ;
	shell->exit_status = exec(shell, ast, argv0);
	free_ast(ast);
}

static void	main_loop(t_minishell *shell, char *argv0)
{
	int		status;
	char	*input;
	char	*prompt;

	while (1)
	{
		init_interactive_signals();
		status = build_prompt(env, &prompt);
		if (status)
			ft_putendl_fd("minishell: malloc: allocation failed", 2);
		input = readline(prompt);
		if (status != 2)
			free(prompt);
		if (!input)
		{
			if (isatty(STDIN_FILENO))
				ft_putstr_fd("exit\n", STDERR_FILENO);
			break ;
		}
		if (*input)
			process_input(shell, input, argv0);
		free(input);
	}
}

static void	init_minishell(t_minishell *shell, char **envp, char *argv0)
{
	shell->exported_count = 0;
	shell->env = NULL;
	shell->exported = NULL;
	shell->pid = NULL;
	shell->last_pid = NULL;
	shell->exit_status = 0;
	if (build_env(shell, envp, argv0))
	{
		ft_putstr_fd("minishell: shell-init: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		exit_shell(shell, NULL, 0, 1);
	}
	init_token_type_table(shell->token_type_table);
}

int	main(int argc, char **argv, char **envp)
{
	t_minishell	shell;
	size_t		i;

	if (argc != 1)
	{
		ft_putendl_fd("minishell: too many arguments", STDERR_FILENO);
		return (1);
	}
	init_minishell(&shell, envp, argv[0]);
	main_loop(shell);
	return (0);
}

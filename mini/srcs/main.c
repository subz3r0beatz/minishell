/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 17:46:29 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/03 03:59:03 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	process_input(t_minishell *shell, char *input)
{
	add_history(input);
	shell->tokens = lexer(input, shell->token_type_table);
	if (!shell->tokens)
		return ;
	shell->ast = parser(shell->tokens);
	if (!shell->ast)
		return ;
	shell->exit_status = exec(shell, shell->ast);
	free_ast(shell->ast);
}

static void	main_loop(t_minishell *shell)
{
	int		status;
	char	*input;
	char	*prompt;

	while (1)
	{
		status = 0;
		prompt = NULL;
		init_interactive_signals();
		if (isatty(STDIN_FILENO))
		{
			status = build_prompt(shell->env, &prompt);
			if (status)
				ft_putendl_fd("minishell: malloc: allocation failed", 2);
		}
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
			process_input(shell, input);
		free(input);
	}
}

static void	init_minishell(t_minishell *shell, char **argv, char **envp)
{
	shell->exported_count = 0;
	shell->env = NULL;
	shell->exported = NULL;
	shell->tokens = NULL;
	shell->ast = NULL;
	shell->argv0 = argv[0];
	shell->pid = NULL;
	shell->last_pid = NULL;
	shell->exit_status = 0;
	if (build_env(shell, envp, argv[0]))
	{
		ft_putstr_fd("minishell: shell-init: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		exit_shell(shell, 1);
	}
	init_token_type_table(shell->token_type_table);
	init_exec_func_table(shell->exec_func_table);
	init_builtin_func_table(shell->builtin_func_table);
}

int	main(int argc, char **argv, char **envp)
{
	t_minishell	shell;

	if (argc != 1)
	{
		ft_putendl_fd("minishell: too many arguments", STDERR_FILENO);
		return (1);
	}
	init_minishell(&shell, argv, envp);
	main_loop(&shell);
	return (shell.exit_status);
}

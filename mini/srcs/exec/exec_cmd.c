/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmd.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 16:13:55 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/31 17:08:18 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_builtin(char *cmd)
{
	if (cmd[0] == 'e')
	{
		if (ft_strcmp(cmd, "echo") == 0)
			return (1);
		if (ft_strcmp(cmd, "exit") == 0)
			return (2);
		if (ft_strcmp(cmd, "env") == 0)
			return (3);
		if (ft_strcmp(cmd, "export") == 0)
			return (4);
		return (0);
	}
	if (ft_strcmp(cmd, "unset") == 0)
		return (5);
	if (ft_strcmp(cmd, "cd") == 0)
		return (6);
	if (ft_strcmp(cmd, "pwd") == 0)
		return (7);
	return (0);
}

int	exec_builtin(t_minishell *shell, t_ast_node *node, int builtin)
{
	int	saved_stdin;
	int	saved_stdout;
	int	status;

	saved_stdin = dup(STDIN_FILENO);
	saved_stdout = dup(STDOUT_FILENO);
	if (apply_redirections(node->redir))
	{
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdin);
		close(saved_stdout);
		shell->exit_status = 1;
		return (1);
	}
	status = shell->builtin_func_table[builtin - 1](shell, node->args);
}

int	exec_child_binary(t_minishell *shell, t_ast_node *node)
{
}

int	exec_cmd(t_minishell *shell, t_ast_node *node, char *argv0)
{
	int	builtin;

	if (!node)
		return (shell->exit_status);
	if (expand(shell, node, argv0) != 0)
	{
		shell->exit_status = 1;
		return (1);
	}
	if (!node->args || !node->args[0])
	{
		if (apply_redirections(node->redir))
		{
			shell->exit_status = 1;
			return (1);
		}
		shell->exit_status = 0;
		return (0);
	}
	builtin = is_builtin(node->args[0]);
	if (builtin)
		return (exec_builtin(shell, node, builtin));
	return (exec_child_binary(shell, node));
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_builtin.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 16:03:48 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 03:23:44 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_builtin(char *cmd)
{
	if (cmd[0] == 'e')
	{
		if (ft_strcmp(cmd, "echo") == 0)
			return (1);
		if (ft_strcmp(cmd, "env") == 0)
			return (2);
		if (ft_strcmp(cmd, "exit") == 0)
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

static int	has_in_redir(t_redir *redir)
{
	while (redir)
	{
		if (redir->type == TOKEN_LESS || redir->type == TOKEN_DLESS
			|| redir->type == TOKEN_TLESS)
			return (1);
		redir = redir->next;
	}
	return (0);
}

static int	has_out_redir(t_redir *redir)
{
	while (redir)
	{
		if (redir->type == TOKEN_GREAT || redir->type == TOKEN_DGREAT)
			return (1);
		redir = redir->next;
	}
	return (0);
}

int	init_saved_std(t_minishell *shell, t_redir *redir,
	int *in, int *out)
{
	*in = -1;
	*out = -1;
	if (has_in_redir(redir))
	{
		*in = dup(STDIN_FILENO);
		if (*in < 0)
		{
			perror("minishell: dup");
			shell->exit_status = 1;
			return (1);
		}
	}
	if (has_out_redir(redir))
	{
		*out = dup(STDOUT_FILENO);
		if (*out < 0)
		{
			perror("minishell: dup");
			if (*in >= 0)
				close(*in);
			shell->exit_status = 1;
			return (1);
		}
	}
	return (0);
}

int	exec_builtin(t_minishell *shell, t_ast_node *node, int builtin)
{
	int	saved_stdin;
	int	saved_stdout;
	int	status;

	if (!node->redir)
		return (shell->builtin_func_table[builtin - 1](shell, node->args));
	if (init_saved_std(shell, node->redir, &saved_stdin, &saved_stdout))
		return (1);
	if (apply_redirections(shell, node->redir))
	{
		restore_fds(saved_stdin, saved_stdout);
		shell->exit_status = 1;
		return (1);
	}
	if (builtin == 2)
	{
		if (restore_fds(saved_stdin, saved_stdout))
			return (1);
		return (shell->builtin_func_table[builtin - 1](shell, node->args));
	}
	status = shell->builtin_func_table[builtin - 1](shell, node->args);
	if (restore_fds(saved_stdin, saved_stdout))
		return (1);
	return (status);
}

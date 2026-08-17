/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_pipe.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 16:50:30 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 02:38:48 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	common_error(void)
{
	init_interactive_signals();
	ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
	return (1);
}

static int	wait_exec(t_minishell *shell, pid_t left_pid, pid_t right_pid, int pfd[2])
{
	int	status;

	close(pfd[0]);
	close(pfd[1]);
	waitpid(left_pid, NULL, 0);
	waitpid(right_pid, &status, 0);
	init_interactive_signals();
	if (WIFEXITED(status))
		shell->exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
	{
		shell->exit_status = 128 + WTERMSIG(status);
		if (WTERMSIG(status) == SIGINT)
			g_signal_status = 130;
	}
	return (shell->exit_status);
}

static int	do_left(t_minishell *shell, t_ast_node *node,
	int pfd[2], pid_t left_pid)
{
	int	status;

	if (left_pid < 0)
	{
		common_error();
		close(pfd[0]);
		close(pfd[1]);
		return (1);
	}
	if (left_pid == 0)
	{
		shell->is_child = 1;
		close(pfd[0]);
		if (dup2(pfd[1], STDOUT_FILENO) < 0)
		{
			close(pfd[1]);
			exit_shell(shell, 1);
		}
		close(pfd[1]);
		status = exec(shell, node->left);
		exit_shell(shell, status);
	}
	return (0);
}

static int	do_right(t_minishell *shell, t_ast_node *node,
	int pfd[2], pid_t right_pid)
{
	int	status;

	if (right_pid == 0)
	{
		shell->is_child = 1;
		close(pfd[1]);
		if (dup2(pfd[0], STDIN_FILENO) < 0)
		{
			close(pfd[0]);
			exit_shell(shell, 1);
		}
		close(pfd[0]);
		status = exec(shell, node->right);
		exit_shell(shell, status);
	}
	return (0);
}

int	exec_pipe(t_minishell *shell, t_ast_node *node)
{
	int		pfd[2];
	pid_t	left_pid;
	pid_t	right_pid;

	if (pipe(pfd) < 0)
	{
		ft_putstr_fd("minishell: exec: pipe failed\n", STDERR_FILENO);
		return (1);
	}
	init_ignore_signals(1);
	left_pid = fork();
	if (do_left(shell, node, pfd, left_pid))
		return (1);
	right_pid = fork();
	if (right_pid < 0)
	{
		close(pfd[0]);
		close(pfd[1]);
		waitpid(left_pid, NULL, 0);
		return (common_error());
	}
	if (do_right(shell, node, pfd, right_pid))
		return (1);
	return (wait_exec(shell, left_pid, right_pid, pfd));
}

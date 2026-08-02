/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_pipe.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 16:50:30 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 17:43:31 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	left_fork_error(t_minishell *shell, int pfd[2])
{
	init_interactive_signals();
	ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
	shell->exit_status = 1;
	close(pfd[0]);
	close(pfd[1]);
	return (1);
}

int	right_fork_error(t_minishell *shell, pid_t left_pid, int pfd[2])
{
	close(pfd[0]);
	close(pfd[1]);
	waitpid(left_pid, NULL, 0);
	init_interactive_signals();
	ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
	shell->exit_status = 1;
	return (1);
}

void	exec_left_child(t_minishell *shell, t_ast_node *node,
	int pfd[2])
{
	init_child_signals();
	close(pfd[1]);
	if (dup2(pfd[0], STDOUT_FILENO) < 0)
	{
		close(pfd[0]);
		exit_shell(shell, 1);
	}
	close(pfd[0]);
	exit_shell(shell, exec(shell, node->left));
}

void	exec_right_child(t_minishell *shell, t_ast_node *node,
	int pfd[2])
{
	init_child_signals();
	close(pfd[0]);
	if (dup2(pfd[1], STDIN_FILENO) < 0)
	{
		close(pfd[1]);
		exit_shell(shell, 1);
	}
	close(pfd[1]);
	exit_shell(shell, exec(shell, node->right));
}

int	wait_exec(t_minishell *shell, pid_t left_pid, pid_t right_pid, int pfd[2])
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
		if (shell->exit_status - 128 == SIGINT)
			ft_putstr_fd("\n", STDERR_FILENO);
		if (shell->exit_status - 128 == SIGQUIT)
			ft_putstr_fd("Quit\n", STDERR_FILENO);
	}
	return (shell->exit_status);
}

int	exec_pipe(t_minishell *shell, t_ast_node *node)
{
	int		pfd[2];
	pid_t	left_pid;
	pid_t	right_pid;

	if (!node || pipe(pfd) < 0)
	{
		ft_putstr_fd("minishell: exec: pipe failed\n", STDERR_FILENO);
		shell->exit_status = 1;
		return (1);
	}
	init_ignore_signals();
	left_pid = fork();
	if (left_pid < 0)
		return (left_fork_error(shell, pfd));
	if (left_pid == 0)
		exec_left_child(shell, node, pfd);
	right_pid = fork();
	if (right_pid < 0)
		return (right_fork_error(shell, left_pid, pfd));
	if (right_pid == 0)
		exec_right_child(shell, node, pfd);
	return (wait_exec(shell, left_pid, right_pid, pfd));
}

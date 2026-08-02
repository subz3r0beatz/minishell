/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmd.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/31 16:13:55 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 17:45:15 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*malloc_error(void)
{
	ft_putstr_fd("minishell: exec: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (NULL);
}

char	*command_error(char *cmd, int exists)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(cmd, STDERR_FILENO);
	if (exists == 1)
		ft_putstr_fd(": command not found\n", STDERR_FILENO);
	else
		ft_putstr_fd(": Permission denied\n", STDERR_FILENO);
	return (NULL);
}

static char	*check_path(char **split, char *cmd, int *exists)
{
	char	*tmp;
	char	*path;
	size_t	i;

	i = 0;
	while (split[i])
	{
		tmp = ft_strjoin(split[i], "/");
		if (!tmp)
			return (malloc_error());
		path = ft_strjoin(tmp, cmd);
		free(tmp);
		if (!path)
			return (malloc_error());
		if (access(path, F_OK) == 0)
		{
			*exists = 2;
			if (access(path, X_OK) == 0)
				return (path);
		}
		free(path);
		i++;
	}
	return (command_error(cmd, *exists));
}

static char	*get_path(t_minishell *shell, char *cmd, int *exists)
{
	char	*path;
	char	**split;

	if (ft_strchr(cmd, '/'))
	{
		path = ft_strdup(cmd);
		if (!path)
			return (malloc_error());
		return (path);
	}
	get_var_value(shell, "PATH", &path);
	if (path && path[0] && check_exported(shell, "PATH"))
	{
		*exists = 1;
		split = ft_split(path, ':');
		if (!split)
			return (malloc_error());
		path = check_path(split, cmd, exists);
		ft_free_matrix(split, ft_memlen(split, sizeof(char *)));
		if (!path)
			return (NULL);
		return (path);
	}
	path = ft_strdup(cmd);
	if (!path)
		return (malloc_error());
	return (path);
}

void exec_binary_child(t_minishell *shell, t_ast_node *node)
{
	char	*path;
	int		exists;
	int		err;

	init_child_signals();
	if (apply_redirections(node->redir))
		exit_shell(shell, 1);
	exists = 0;
	if (!node->args[0][0])
		exit_shell(shell, 127);
	path = get_path(shell, node->args[0], &exists);
	if (!path)
	{
		if (exists == 1)
			exit_shell(shell, 127);
		else if (exists == 2)
			exit_shell(shell, 126);
		exit_shell(shell, 1);
	}
	if (!shell->exported)
		shell->exported = env_to_matrix(shell);
	if (!shell->exported)
	{
		free(path);
		ft_putstr_fd("minishell: exec: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		exit_shell(shell, 1);
	}
	execve(path, node->args, shell->exported);
	err = errno;
	free(path);
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	errno = err;
	perror(node->args[0]);
	if (errno == ENOENT)
		exit_shell(shell, 127);
	exit_shell(shell, 126);
}

static int	exec_binary(t_minishell *shell, t_ast_node *node)
{
	pid_t	pid;
	int		status;

	init_ignore_signals();
	pid = fork();
	if (pid < 0)
	{
		init_interactive_signals();
		ft_putstr_fd("minishell: exec: fork failed\n", STDERR_FILENO);
		shell->exit_status = 1;
		return (1);
	}
	if (pid == 0)
		exec_binary_child(shell, node);
	waitpid(pid, &status, 0);
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

int	exec_cmd(t_minishell *shell, t_ast_node *node)
{
	int	builtin;

	if (!node)
		return (shell->exit_status);
	if (expand(shell, node) != 0)
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
	return (exec_binary(shell, node));
}

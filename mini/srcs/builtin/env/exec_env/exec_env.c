/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 10:33:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/13 21:09:35 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	exit_env(char ***matrices[2], t_flags *flags,
	size_t lens[2], int exit_code)
{
	if (!matrices)
		exit(exit_code);
	if (matrices[0] && *matrices[0])
		ft_free_matrix(*matrices[0], lens[1]);
	if (matrices[1] && *matrices[1])
		ft_free_matrix(*matrices[1], lens[0]);
	if (!flags)
		exit(exit_code);
	if (flags->custom_argv0)
		free(flags->custom_argv0);
	if (flags->chdir_path)
		free(flags->chdir_path);
	exit(exit_code);
}

static void	switch_dir(char	***matrices[2], t_flags *flags, size_t lens[2])
{
	if (flags->chdir_path && chdir(flags->chdir_path))
	{
		ft_putstr_fd("minishell: env: "
			"cannot change directory to '", STDERR_FILENO);
		ft_putstr_fd(flags->chdir_path, STDERR_FILENO);
		perror("'");
		exit_env(matrices, flags, lens, 125);
	}
}

static int	exec_error(char *cmd)
{
	free(cmd);
	ft_putstr_fd("minishell: env: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (125);
}

static int	exec_cmd(char **args, char **exported, t_flags *flags,
		size_t lens[2])
{
	int		exit_code;
	char	*cmd;

	cmd = resolve_cmd_path(char args, exported);
	if (!cmd)
		return (malloc_error(NULL));
	if (flags->custom_argv0)
	{
		free(args[0]);
		args[0] = ft_strdup(flags->custom_argv0);
		if (!args[0])
			return (malloc_error(cmd));
	}
	if (execve(cmd, args, exported))
	{
		ft_putstr_fd("minishell: env: '", STDERR_FILENO);
		ft_putstr_fd(cmd, STDERR_FILENO);
		perror("'");
		free(cmd);
		if (errno == EACCES || errno == ENOEXEC)
			return (126);
		return (127);
	}
	free(cmd);
	return (0);
}

int	exec_env(char **args, char **exported, t_flags *flags, size_t lens[2])
{
	pid_t	pid;
	int		status;
	char	***matrices[2];

	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd("minishell: env: fork failed\n", STDERR_FILENO);
		return (125);
	}
	if (pid == 0)
	{
		matrices[0] = &args;
		matrices[1] = &exported;
		switch_dir(matrices, flags, lens);
		exit_env(matrices, flags, lens,
			exec_cmd(args, exported, flags, lens));
	}
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (0);
}

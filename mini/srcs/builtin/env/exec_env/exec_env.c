/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 10:33:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 17:30:01 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	switch_dir(char	**matrices[2],
	t_flags *flags, t_max_uints *max_uints)
{
	if (flags->chdir_path && chdir(flags->chdir_path))
	{
		ft_putstr_fd("minishell: env: "
			"cannot change directory to '", STDERR_FILENO);
		ft_putstr_fd(flags->chdir_path, STDERR_FILENO);
		perror("'");
		exit(exit_env(matrices, flags, max_uints, 125));
	}
}

static int	malloc_error(char *cmd)
{
	free(cmd);
	ft_putstr_fd("minishell: env: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (125);
}

static int	exec_cmd(char **args, char **exported, char *cmd)
{
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

static int	parse_cmd(char **matrices[2],
	t_flags *flags, t_max_uints *max_uints)
{
	int		exit_code;
	int		no_malloc_error;
	char	*cmd;

	exit_code = 0;
	cmd = resolve_cmd_path(&matrices[1][max_uints->i],
			matrices[0], &exit_code, &no_malloc_error);
	if (!cmd && !no_malloc_error)
		return (malloc_error(NULL));
	if (exit_code)
	{
		free(cmd);
		return (exit_code);
	}
	if (flags->custom_argv0)
	{
		free(matrices[1][max_uints->i]);
		matrices[1][max_uints->i] = ft_strdup(flags->custom_argv0);
		if (!matrices[1][max_uints->i])
			return (malloc_error(cmd));
	}
	return (exec_cmd(&matrices[1][max_uints->i], matrices[0], cmd));
}

int	exec_env(char **matrices[2], t_flags *flags, t_max_uints *max_uints)
{
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid < 0)
	{
		ft_putstr_fd("minishell: env: fork failed\n", STDERR_FILENO);
		return (125);
	}
	if (pid == 0)
	{
		switch_dir(matrices, flags, max_uints);
		exit(exit_env(matrices, flags, max_uints,
				parse_cmd(matrices, flags, max_uints)));
	}
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (0);
}

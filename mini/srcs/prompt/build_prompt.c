/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_prompt.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 19:20:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 23:07:17 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*build_username(t_minishell *shell, size_t *size, int *malloc_error)
{
	char	*username;
	int		error;

	error = 1;
	username = get_username(shell, &error);
	if (username)
		*size = *size - 7 + ft_strlen(username);
	else if (error)
		*malloc_error = 1;
	return (username);
}

static char	*build_hostname(t_minishell *shell, size_t *size, int *malloc_error)
{
	char	*hostname;
	int		error;

	error = 1;
	hostname = get_hostname(shell, &error);
	if (hostname)
		*size = *size - 7 + ft_strlen(hostname);
	else if (error)
		*malloc_error = 1;
	return (hostname);
}

static char	*build_pwd(t_minishell *shell, size_t *size, int *malloc_error)
{
	char	*pwd;
	int		error;

	error = 1;
	pwd = get_prompt_pwd(shell, &error);
	if (pwd)
		*size += ft_strlen(pwd);
	else if (error)
		*malloc_error = 1;
	return (pwd);
}

static char	*cpy_prompt(char *username, char *hostname, char *pwd, size_t *size)
{
	char	*prompt;

	prompt = malloc(sizeof(char) * *size);
	if (prompt)
	{
		if (username)
			ft_strlcpy(prompt, username, *size);
		else
			ft_strlcpy(prompt, "unknown", *size);
		ft_strlcat(prompt, "@", *size);
		if (hostname)
			ft_strlcat(prompt, hostname, *size);
		else
			ft_strlcat(prompt, "unknown", *size);
		ft_strlcat(prompt, ":", *size);
		if (pwd)
			ft_strlcat(prompt, pwd, *size);
		ft_strlcat(prompt, "$\n", *size);
	}
	free(username);
	free(hostname);
	free(pwd);
	return (prompt);
}

void	build_prompt(t_minishell *shell)
{
	int		malloc_error;
	size_t	size;
	char	*prompt;

	if (!isatty(STDIN_FILENO))
		return ;
	size = 19;
	malloc_error = 0;
	prompt = cpy_prompt(build_username(shell, &size, &malloc_error),
										 build_hostname(shell, &size, &malloc_error),
										 build_pwd(shell, &size, &malloc_error),
										 &size);
	if (malloc_error || !prompt)
		ft_putstr_fd("minishell: malloc: cannot allocate memory\n", STDERR_FILENO);
	if (prompt)
		write(STDOUT_FILENO, prompt, size - 1);
	free(prompt);
}

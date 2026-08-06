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

static int	build_username(t_robin *env, char **username, size_t *size)
{
	char	buffer[32768];
	int		no_malloc_error;

	no_malloc_error = 0;
	*username = get_username(env, buffer, &no_malloc_error);
	if (!*username && !no_malloc_error)
		return (1);
	if (!*username)
		return (0);
	*size = *size - 7 + ft_strlen(*username);
	return (0);
}

static int	build_hostname(t_robin *env, char **hostname, size_t *size)
{
	char	buffer[HOST_NAME_MAX];
	int		no_malloc_error;

	no_malloc_error = 0;
	*hostname = get_hostname(env, buffer, &no_malloc_error);
	if (!*hostname && !no_malloc_error)
		return (1);
	if (!*hostname)
		return (0);
	*size = *size - 7 + ft_strlen(*hostname);
	return (0);
}

static int	build_pwd(t_robin *env, char **pwd, size_t *size)
{
	char	buffer[PATH_MAX];
	int		no_malloc_error;

	no_malloc_error = 1;
	*pwd = get_prompt_pwd(env, buffer, &no_malloc_error);
	if (!pwd && !no_malloc_error)
		return (1);
	if (!*pwd)
		return (0);
	*size = *size + ft_strlen(*pwd);
	if (!no_malloc_error)
		return (1);
	return (0);
}

static char	*cpy_prompt(char *username, char *hostname, char *pwd, size_t size)
{
	char	*prompt;

	prompt = malloc(sizeof(char) * size);
	if (!prompt)
		return (NULL);
	if (!username)
		username = "unknown";
	if (!hostname)
		hostname = "unknown";
	if (!pwd)
		pwd = "";
	ft_strlcpy(prompt, username, size);
	ft_strlcat(prompt, "@", size);
	ft_strlcat(prompt, hostname, size);
	ft_strlcat(prompt, ":", size);
	ft_strlcat(prompt, pwd, size);
	ft_strlcat(prompt, "$\n$ ", size);
	return (prompt);
}

int	build_prompt(t_robin *env, char **prompt)
{
	int		status;
	size_t	size;
	char	*username;
	char	*hostname;
	char	*pwd;

	*prompt = NULL;
	status = 0;
	size = 21;
	if (build_username(env, &username, &size)
		|| build_hostname(env, &hostname, &size) || build_pwd(env, &pwd, &size))
		status = 1;
	*prompt = cpy_prompt(username, hostname, pwd, size);
	free(username);
	free(hostname);
	free(pwd);
	if (!*prompt)
	{
		*prompt = "unknown@unknown:$\n$ ";
		status = 2;
	}
	if (status)
		ft_putstr_fd("minishell: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	return (status);
}

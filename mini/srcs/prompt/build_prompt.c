/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_prompt.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 19:20:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 13:59:24 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	build_username(t_robin *env, char **username, size_t *size)
{
	char	buffer[8192];

	*username = get_username(env, buffer);
	if (!*username)
		return ;
	*size = *size - 7 + ft_strlen(*username);
}

static void	build_hostname(t_robin *env, char **hostname, size_t *size)
{
	char	buffer[8192];

	*hostname = get_hostname(env, buffer);
	if (!*hostname)
		return ;
	*size = *size - 7 + ft_strlen(*hostname);
}

static void	build_pwd(t_robin *env, char **pwd, size_t *size)
{
	char	buffer[8192];

	*pwd = get_pwd(env, buffer);
	if (!*pwd)
		return ;
	*size = *size + ft_strlen(*pwd);
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

	status = 0;
	size = 21;
	build_username(env, &username, &size);
	build_hostname(env, &hostname, &size);
	build_pwd(env, &pwd, &size);
	if (!username || !hostname || !pwd)
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
	return (status);
}

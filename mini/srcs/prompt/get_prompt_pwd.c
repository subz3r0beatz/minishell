/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_prompt_pwd.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 01:34:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/03 03:31:11 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*get_raw_cwd(t_minishell *shell, int *malloc_error)
{
	char	*pwd;
	char	buffer[PATH_MAX];

	pwd = NULL;
	if (!get_var_value(shell, "PWD", &pwd) && pwd)
		return (ft_strdup(pwd));
	else if (getcwd(buffer, PATH_MAX))
		return (ft_strdup(buffer));
	else
		perror("minishell: pwd: error retrieving current directory: "
			"getcwd: cannot access parent directories");
	*malloc_error = 0;
	return (pwd);
}

static char	*clean_slashes(char *s)
{
	char	*clean;
	size_t	i;
	size_t	len;

	if (!s)
		return (NULL);
	i = -1;
	len = 0;
	while (s[++i])
		if (s[i] != '/' || s[i + 1] != '/')
			len++;
	clean = malloc(len + 1 * sizeof(char));
	if (!clean)
		return (NULL);
	clean[len] = 0;
	i = -1;
	len = 0;
	while (s[++i])
		if (s[i] != '/' || s[i + 1] != '/')
			clean[len++] = s[i];
	return (clean);
}

static char	*handle_prompt_home(char *home, char *pwd)
{
	char	*tmp_pwd;
	size_t	home_len;
	int		offset;

	tmp_pwd = clean_slashes(pwd);
	if (!tmp_pwd)
	{
		free(home);
		free(pwd);
		return (NULL);
	}
	home_len = ft_strlen(home);
	offset = (home_len == 1);
	if (home_len && tmp_pwd && !ft_strncmp(home, tmp_pwd, home_len)
		&& (tmp_pwd[home_len - offset] == '/' || !tmp_pwd[home_len - offset]))
	{
		free(pwd);
		if (tmp_pwd[0] && tmp_pwd[1])
			pwd = ft_strjoin("~", tmp_pwd + home_len - offset);
		else
			pwd = ft_strdup("~");
	}
	free(tmp_pwd);
	free(home);
	return (pwd);
}

char	*get_prompt_pwd(t_minishell *shell, int *malloc_error)
{
	char			*pwd;
	char			*home;
	char			*tmp_home;

	pwd = get_raw_cwd(shell, malloc_error);
	if (!pwd)
		return (NULL);
	if (get_var_value(shell, "HOME", &tmp_home) || !tmp_home || !*tmp_home)
		tmp_home = search_passwd(5, malloc_error);
	else
		tmp_home = ft_strdup(tmp_home);
	home = clean_slashes(tmp_home);
	free(tmp_home);
	if (!home)
		return (pwd);
	return (handle_prompt_home(home, pwd));
}

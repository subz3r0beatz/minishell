/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   canonalize_path.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 19:31:13 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/02 19:42:55 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*canonalize_path(t_minishell *shell, char *pwd, char *path)
{
	char	*ret;
	size_t	len;
	size_t	i;
	size_t	prev;

	len = ft_strlen(pwd) + ft_strlen(path) + 1;
	ret = malloc(len);
	if (!ret)
		return (NULL);
	i = 0;
	prev = 0;
	while (pwd[i])
	{
		ret[i] = pwd[i];
		i++;
	}
	while (path[prev])
		ret[i++] = path[prev++];
	ret[i] = '\0';
	i = 0;
	while (pwd[i] || path[i])
	{
		if (path[i] == '/' && path[i + 1] == '/')
		i++;
	}
	return (ret);
}

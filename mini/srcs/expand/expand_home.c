/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_home.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 22:35:26 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/26 16:33:36 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*expand_home(t_minishell *shell, char *word)
{
	int		malloc_error;
	char	*home;
	char	*tmp;

	malloc_error = 1;
	if (get_var_value(shell, "HOME", &home) || !home)
		home = search_passwd(5, &malloc_error);
	else
		home = ft_strdup(home);
	if (!home)
	{
		free(word);
		return (NULL);
	}
	if (ft_strchr(word, '/'))
	{
		tmp = ft_strjoin(home, word + 1);
		free(home);
		free(word);
		return (tmp);
	}
	free(word);
	return (home);
}

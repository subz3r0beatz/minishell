/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   add_variables.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 16:00:16 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/09 17:58:23 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <stddef.h>

static int	malloc_error(void)
{
	ft_putstr_fd("minishell: env: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (1);
}

static int	lookup_var(char **exported, char *arg, char *ptr, size_t len)
{
	size_t	i;

	i = 0;
	while (exported[i])
	{
		if (!ft_strncmp(exported[i], arg, arg - ptr))
		{
			free(exported[i]);
			exported[i] = ft_strdup(arg);
			if (!exported[i])
			{
				exported[i] = NULL;
				return (malloc_error());
			}
			return (0);
		}
		i++;
	}
	return (2);
}

static int	add_var(char ***exported, char **args, size_t *len, size_t *capacity)
{
	char	**new_exported;
	size_t	i;

	if (*len == capacity)
	{
		i = 0;
		while (args[i] && ft_strchr(args[i], '='))
			i++;
		new_exported = ft_realloc(*exported, capacity + i + 1, sizeof(char *));
		if (!new_exported)
			return (malloc_error());
		*exported = new_exported;
		*capacity += i;
	}
	*exported[*len] = ft_strdup(args[0]);
	if (!(*exported)[*len])
		return (malloc_error());
	(*len)++;
	return (0);
}

size_t	add_variables(t_minishell *shell, char **args, char ***exported,
	size_t *max_uints[2])
{
	char	*ptr;
	size_t	i;
	size_t	*len;
	size_t	capacity;
	int		status;

	i = max_uints[0];
	len = (*max_uints)[1];
	capacity = shell->exported_count;
	while (args[i])
	{
		ptr = ft_strchr(args[i], '=');
		if (!ptr)
			return (i);
		status = lookup_var(*exported, args[i], ptr, len);
		if (status == 1)
			return (0);
		if (status == 2)
			if (add_var(exported, &args[i], len, *capacity))
				return (0);
		i++;
	}
	return (i);
}

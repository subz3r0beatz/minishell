/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   add_variables.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 16:00:16 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/17 02:58:07 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	malloc_error(void)
{
	ft_putstr_fd("minishell: env: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (1);
}

static int	lookup_var(char **matrices[2], t_max_uints *max_uints, char *ptr)
{
	size_t	i;

	i = 0;
	while (matrices[0][i])
	{
		if (!ft_strncmp(matrices[0][i],
			matrices[1][max_uints->i], ptr - matrices[1][max_uints->i])
			&& matrices[0][i][ptr - matrices[1][max_uints->i]] == '=')
		{
			free(matrices[0][i]);
			matrices[0][i] = ft_strdup(matrices[1][max_uints->i]);
			if (!matrices[0][i])
				return (malloc_error());
			return (0);
		}
		i++;
	}
	return (2);
}

static int	add_var(char **matrices[2], t_max_uints *max_uints)
{
	char	**new_exported;
	size_t	i;

	if (max_uints->exported_len == max_uints->capacity)
	{
		i = max_uints->i;
		while (matrices[1][i] && ft_strchr(matrices[1][i], '='))
			i++;
		new_exported = ft_realloc(matrices[0],
				max_uints->capacity + i + 1 - max_uints->i, sizeof(char *));
		if (!new_exported)
			return (malloc_error());
		matrices[0] = new_exported;
		max_uints->capacity += i - max_uints->i;
	}
	matrices[0][max_uints->exported_len] = ft_strdup(matrices[1][max_uints->i]);
	if (!matrices[0][max_uints->exported_len])
		return (malloc_error());
	max_uints->exported_len++;
	matrices[0][max_uints->exported_len] = NULL;
	return (0);
}

size_t	add_variables(char **matrices[2], t_max_uints *max_uints)
{
	char	*ptr;
	size_t	*i;
	int		status;

	i = &max_uints->i;
	while (matrices[1][*i])
	{
		ptr = ft_strchr(matrices[1][*i], '=');
		if (!ptr)
			return (*i);
		status = lookup_var(matrices, max_uints, ptr);
		if (status == 1)
			return (0);
		if (status == 2)
			if (add_var(matrices, max_uints))
				return (0);
		(*i)++;
	}
	return (*i);
}

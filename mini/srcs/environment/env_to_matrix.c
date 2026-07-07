/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_to_matrix.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 16:12:09 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 20:39:27 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*create_var(char *key, char *value)
{
	char	*str;
	char	*tmp;

	if (value)
	{
		tmp = ft_strjoin(key, "=");
		if (!tmp)
			return (NULL);
		str = ft_strjoin(tmp, value);
		free(tmp);
	}
	else
		str = ft_strdup(key);
	if (!str)
		return (NULL);
	return (str);
}

static char	**fill_matrix(t_robin *env, char **matrix)
{
	size_t	i;
	t_env	*value;

	i = 0;
	while (i < robin->capacity)
	{
		value = (t_env *)robin->data[i]->value;
		if (robin->ctrl[i] > 0 && value->is_exported)
		{
			matrix[i] = create_var(value->key, value->value);
			if (!matrix[i])
			{
				ft_free_matrix(matrix, i);
				return (NULL);
			}
		}
		i++;
	}
	return (matrix);
}

char	**env_to_matrix(t_minishell *shell)
{
	char	**matrix;

	matrix = malloc(sizeof(char *) * shell->exported_count);
	if (!matrix)
		return (NULL);
	matrix = fill_matrix(shell->env, matrix);
	return (matrix);
}

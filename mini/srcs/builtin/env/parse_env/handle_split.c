/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_split.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 17:12:11 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/21 23:53:24 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	**malloc_error(void)
{
	ft_putstr_fd("minishell: env: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (NULL);
}

static char	**add_new_args(char	**args, char *string,
	size_t i, size_t *args_size)
{
	char	**split;
	char	**new_args;
	size_t	split_size;

	split = ft_char_split_quotes_exl(string, ft_iswhite);
	if (!split)
		return (malloc_error());
	split_size = ft_memlen(split, sizeof(char *));
	new_args = ft_realloc(args, split_size + *args_size + 1, sizeof(char *));
	if (!new_args)
	{
		ft_free_matrix(split, split_size);
		return (malloc_error());
	}
	ft_memmove(&new_args[split_size + i + 1], &new_args[i + 1],
		(*args_size - i) * sizeof(char *));
	ft_memmove(&new_args[i + 1], split, split_size * sizeof(char *));
	free(split);
	*args_size += split_size;
	return (new_args);
}

int	handle_split(char ***args, size_t *i, size_t *j, size_t *args_size)
{
	char	*string;
	char	**new_args;

	(*j)++;
	if (!(*args)[*i][*j] && !(*args)[*i + 1])
	{
		ft_putstr_fd("minishell: env: option requires an argument -- 'S'\n"
			"Try 'env --help' for more information.\n", STDERR_FILENO);
		return (1);
	}
	if ((*args)[*i][*j])
		string = &(*args)[*i][*j];
	else
		string = (*args)[++*i];
	new_args = add_new_args(*args, string, *i, args_size);
	if (!new_args)
		return (1);
	*args = new_args;
	*j = ft_strlen((*args)[*i]) - 1;
	return (0);
}

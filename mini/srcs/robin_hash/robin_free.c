/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_free.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 22:01:40 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/19 16:11:57 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	robin_free(t_robin *robin)
{
	size_t	i;
	size_t	j;

	if (!robin)
		return ;
	if (robin->ctrl && robin->data)
	{
		i = 0;
		j = 0;
		while (i < robin->capacity && j < robin->count)
		{
			if (robin->ctrl[i])
			{
				j++;
				robin->del_function(robin->data[i].key, robin->data[i].value);
			}
			i++;
		}
	}
	if (robin->ctrl)
		free(robin->ctrl);
	if (robin->data)
		free(robin->data);
	free(robin);
}

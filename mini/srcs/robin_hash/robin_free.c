/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   robin_free.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/01 22:01:40 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/11 16:06:34 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "robin_hash.h"

void	robin_free(t_robin *robin)
{
	size_t	i;

	if (!robin)
		return ;
	i = 0;
	while (i < robin->capacity)
	{
		if (robin->ctrl[i])
			robin->del_function(robin->data[i].key, robin->data[i].value);
		i++;
	}
	free(robin->ctrl);
	free(robin->data);
	free(robin);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   delete_node.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 19:26:56 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 12:38:20 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "environment.h"

int	delete_node(void *key, void *value)
{
	char	*str;
	t_env	*node;

	str = (char *)key;
	node = (t_env *)value;
	free(str);
	free(node->value);
	free(node);
	return (0);
}

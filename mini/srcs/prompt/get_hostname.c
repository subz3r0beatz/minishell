/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_hostname.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 01:05:43 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/03 03:33:38 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*get_hostname(t_minishell *shell, int *malloc_error)
{
	size_t	i;
	ssize_t	bytes_read;
	char	*hostname;
	char	buffer[HOST_NAME_MAX];

	if (!get_var_value(shell, "HOSTNAME", &hostname) && hostname)
		return (ft_strdup(hostname));
	bytes_read = open_read("/etc/hostname", buffer, HOST_NAME_MAX);
	if (bytes_read <= 0)
	{
		*malloc_error = 0;
		return (NULL);
	}
	i = 0;
	while (i < (size_t)bytes_read && buffer[i] != '\n' && buffer[i] != '.')
		i++;
	buffer[i] = '\0';
	return (ft_strdup(buffer));
}

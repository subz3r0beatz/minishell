/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/06 06:51:45 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/18 17:15:09 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	ft_export(t_robin *env, char **args, int fd_out)
{
	int				status;
	size_t			i;

	if (!args[1])
	{
		status = print_export(env, fd_out);
		return (status);
	}
	status = parse_vars(env, args);
	return (status);
}

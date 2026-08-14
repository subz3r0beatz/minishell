/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_username.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 01:01:10 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/03 03:33:14 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*get_username(t_minishell *shell, int *malloc_error)
{
	char			*usr;

	if (!get_var_value(shell, "USER", &usr) && usr)
		return (ft_strdup(usr));
	if (!get_var_value(shell, "LOGNAME", &usr) && usr)
		return (ft_strdup(usr));
	return (search_passwd(0, malloc_error));
}

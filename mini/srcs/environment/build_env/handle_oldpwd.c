/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_oldpwd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 11:38:08 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 02:16:54 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_oldpwd(t_minishell *shell)
{
	char			*oldpwd;

	if (!get_var_value(shell, "OLDPWD", &oldpwd))
		return (0);
	return (insert_new_node(shell, "OLDPWD", NULL, 1));
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_pwd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 10:09:53 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/21 19:46:03 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_pwd(t_minishell *shell)
{
	char	pwd_buf[PATH_MAX];
	int		get_cwd_fail;

	get_cwd_fail = 0;
	if (!getcwd(pwd_buf, PATH_MAX))
	{
		perror("minishell: shell-init: error retrieving current directory: "
			"getcwd: cannot access parent directories");
		get_cwd_fail = 1;
	}
	if (robin_search(shell->env, "PWD"))
		return (0);
	if (get_cwd_fail)
		return (0);
	return (insert_new_node(shell, "PWD", pwd_buf, 1));
}

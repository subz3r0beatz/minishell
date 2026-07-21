/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit_shell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:49:42 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/21 19:56:41 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	exit_shell(t_minishell *shell, char **args, int fd_out, int exit_status)
{
	if (args)
		ft_free_matrix(args, ft_memlen(args, sizeof(char *)));
	robin_free(shell->env);
	close(fd_out);
	exit(exit_status);
}

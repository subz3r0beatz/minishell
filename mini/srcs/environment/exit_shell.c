/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit_shell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 19:49:42 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/02 17:49:56 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	exit_shell(t_minishell *shell, int exit_status)
{
	if (shell->pid)
		free(shell->pid);
	if (shell->last_pid)
		free(shell->last_pid);
	if (shell->exported)
		ft_free_matrix(shell->exported,
			ft_memlen(shell->exported, sizeof(char *)));
	if (shell->ast)
		free_ast(shell->ast);
	if (shell->env)
		robin_free(shell->env);
	rl_clear_history();
	exit(exit_status);
}

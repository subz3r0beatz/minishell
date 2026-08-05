/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_shell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 18:12:34 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/04 18:14:08 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	free_shell(t_minishell *shell, int exit_status)
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
	if (shell->input)
		free(shell->input);
	rl_clear_history();
	return (exit_status);
}

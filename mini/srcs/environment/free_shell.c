/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_shell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 18:12:34 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/18 01:48:52 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	free_shell(t_minishell *shell, int exit_status)
{
	if (shell->env)
		robin_free(shell->env);
	if (shell->exported)
		ft_free_matrix(shell->exported,
			ft_memlen(shell->exported, sizeof(char *)));
	if (shell->input)
		free(shell->input);
	if (shell->history)
		free(shell->history);
	if (shell->tokens)
		free(shell->tokens);
	if (shell->ast)
		free_ast(shell->ast);
	if (shell->pid)
		free(shell->pid);
	if (shell->last_pid)
		free(shell->last_pid);
	rl_clear_history();
	return (exit_status);
}

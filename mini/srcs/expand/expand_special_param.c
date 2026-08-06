/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_special_param.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 23:50:57 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/03 03:53:07 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

size_t	count_special_param_len(t_minishell *shell, char param, size_t *i)
{
	size_t	len;
	int		exit_status;

	++*i;
	if (param == '?')
	{
		exit_status = shell->exit_status;
		len = 0;
		if (exit_status <= 0)
			len++;
		exit_status = (exit_status ^ (exit_status >> 31)) - (exit_status >> 31);
		while (exit_status > 0)
		{
			len++;
			exit_status /= 10;
		}
		return (len);
	}
	else if (param == '0' && shell->argv0)
		return (ft_strlen(shell->argv0));
	else if (param == '$' && shell->pid)
		return (ft_strlen(shell->pid));
	else if (param == '!' && shell->last_pid)
		return (ft_strlen(shell->last_pid));
	return (0);
}

static void	cpy_exit_status(t_minishell *shell, char *new_word, size_t len)
{
	size_t	exit_status;

	exit_status = shell->exit_status;
	exit_status = (exit_status ^ (exit_status >> 31)) - (exit_status >> 31);
	while (len > 0)
	{
		len--;
		new_word[len] = (exit_status % 10) + '0';
		exit_status /= 10;
	}
	if (shell->exit_status < 0)
		new_word[0] = '-';
}

size_t	copy_special_param(t_minishell *shell, char *new_word,
	char param, size_t *i)
{
	size_t	len;

	len = count_special_param_len(shell, param, i);
	if (param == '?')
		cpy_exit_status(shell, new_word, len);
	else if (param == '0' && shell->argv0)
		ft_memcpy(new_word, shell-> argv0, len);
	else if (param == '!' && shell->last_pid)
		ft_memcpy(new_word, shell->last_pid, len);
	else if (param == '$' && shell->pid)
		ft_memcpy(new_word, shell->pid, len);
	return (len);
}

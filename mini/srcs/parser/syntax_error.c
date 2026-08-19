/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   syntax_error.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 18:40:24 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/18 21:05:31 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast_node	*syntax_error(t_minishell *shell, t_token *token)
{
	if (!token)
		ft_putstr_fd("minishell: syntax error: "
			"unexpected end of file\n", STDERR_FILENO);
	else
	{
		ft_putstr_fd("minishell: syntax error near unexpected token `",
			STDERR_FILENO);
		if (!token || !token->value)
			ft_putstr_fd("newline", STDERR_FILENO);
		else
			ft_putstr_fd(token->value, STDERR_FILENO);
		ft_putstr_fd("'\n", STDERR_FILENO);
		if (!isatty(STDIN_FILENO) && shell->input)
		{
			ft_putstr_fd("`", STDERR_FILENO);
			ft_putstr_fd(shell->input, STDERR_FILENO);
			ft_putstr_fd("'\n", STDERR_FILENO);
		}
	}
	shell->exit_status = 2;
	shell->syn_err = 1;
	return (NULL);
}

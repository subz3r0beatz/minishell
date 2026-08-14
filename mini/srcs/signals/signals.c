/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 19:50:54 by fldumas-          #+#    #+#             */
/*   Updated: 2026/08/05 05:02:28 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

volatile sig_atomic_t	g_signal_status = 0;

static void	sig_handler_interactive(int sig)
{
	(void)sig;
	g_signal_status = 130;
}

static void	sig_handler_heredoc(int sig)
{
	(void)sig;
	g_signal_status = 130;
	write(STDOUT_FILENO, "\n", 1);
	close(STDIN_FILENO);
}

int	rl_signal_check(void)
{
	if (g_signal_status == 130)
		rl_done = 1;
	return (0);
}

void	init_interactive_signals(int handler)
{
	struct sigaction	sa_int;
	struct sigaction	sa_quit;

	g_signal_status = 0;
	if (handler == 1)
		sa_int.sa_handler = sig_handler_interactive;
	else
		sa_int.sa_handler = sig_handler_heredoc;
	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_flags = 0;
	sigaction(SIGINT, &sa_int, NULL);
	sa_quit.sa_handler = SIG_IGN;
	sigemptyset(&sa_quit.sa_mask);
	sa_quit.sa_flags = 0;
	sigaction(SIGQUIT, &sa_quit, NULL);
}

void	init_ignore_signals(int ignore)
{
	g_signal_status = 0;
	if (ignore)
	{
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
	}
	else
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
	}
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 19:46:53 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/29 19:49:03 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALS_H
# define SIGNALS_H

extern volatile sig_atomic_t	g_signal_status;

int		rl_signal_check(void);
void	init_interactive_signals(int handler);
void	init_ignore_signals(int ignore);

#endif

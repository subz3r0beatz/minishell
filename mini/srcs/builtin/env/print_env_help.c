/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_env_help.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 17:53:32 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/07 21:09:05 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	print_env_help(int fd_out)
{
	ft_putstr_fd("Usage: env [OPTION]... [-] [NAME=VALUE]... "
		"[COMMAND [ARG]...]\nSet each NAME to VALUE in the environment "
		"and run COMMAND.\n\nMandatory arguments to long options are "
		"mandatory for short options too.\n  -a, --argv0=ARG          pass "
		"ARG as the zeroth argument of COMMAND\n  -i, --ignore-environment "
		"start with an empty environment\n  -0, --null               end each "
		"output line with NUL, not newline\n  -u, --unset=NAME         remove "
		"variable from the environment\n  -C, --chdir=DIR          change "
		"working directory to DIR\n  -S, --split-string=S     process and "
		"split S into separate arguments\n\nA mere - implies -i.\nIf no "
		"COMMAND, print the resulting environment.\n\nExit status:\n  125  "
		"if the env command itself fails\n  126  if COMMAND is found but "
		"cannot be invoked\n  127  if COMMAND cannot be found\n  -    the "
		"exit status of COMMAND otherwise\n", fd_out);
	return (0);
}

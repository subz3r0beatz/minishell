/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 17:46:29 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/20 15:19:40 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	main_loop(t_robin *env)
{
	int		status;
	char	*input;
	char	*prompt;

	while (1)
	{
		status = build_prompt(env, &prompt);
		if (status)
			ft_putendl_fd("minishell: malloc: allocation failed", 2);
		input = readline(prompt);
		if (status != 2)
			free(prompt);
		if (!input)
			break ;
		add_history(input);
		free(input);
	}
}

int	main(int argc, char **argv, char **envp)
{
	t_robin	*env;

	if (argc != 1)
		return (1);
	(void) argv;
	env = build_env(envp);
	if (!env)
		return (1);
	main_loop(env);
	return (0);
}

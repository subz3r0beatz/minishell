/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 17:46:29 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/14 12:27:27 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	main_loop(t_robin *env)
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

static t_minishell	init_minishell(char **envp)
{
	t_minishell	*minishell;
	t_robin		*env;

	minishell = malloc(sizeof(t_minishell));
	if (!minishell)
		return (NULL);
	env = build_env(envp);
	if (!env)
	{
		free(minishell);
		return (NULL);
	}
	minishell->env = env;
	minishell->exported = NULL;
	init_lookup_table(minishell->lookup_table);
	return (minishell);
}

int	main(int argc, char **argv, char **envp)
{
	t_minishell	*minishell;

	if (argc != 1)
		return (1);
	(void) argv;
	minishell = init_minishell(envp);
	if (!minishell)
		return (1);
	main_loop(minishell);
	return (0);
}

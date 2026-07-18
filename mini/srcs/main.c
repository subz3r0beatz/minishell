/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 17:46:29 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/18 06:51:02 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

//static void	main_loop(t_robin *env)
//{
//	int		status;
//	char	*input;
//	char	*prompt;
//
//	while (1)
//	{
//		status = build_prompt(env, &prompt);
//		if (status)
//			ft_putendl_fd("minishell: malloc: allocation failed", 2);
//		input = readline(prompt);
//		if (status != 2)
//			free(prompt);
//		if (!input)
//			break ;
//		add_history(input);
//		free(input);
//	}
//}

static int	init_minishell(t_minishell *shell, char **envp)
{
	shell->exported_count = 0;
	if (build_env(shell, envp))
	{
		robin_free(shell->env);
		return (1);
	}
	shell->exported = NULL;
	//init_token_type_table(shell->token_type_table);
	return (0);
}

static int	parse_flag(char *arg, char *str, size_t *j)
{
	size_t	i;

	i = 0;
	if (!arg[i] || arg[i] == '=')
		return (0);
	while (arg[i] && str[i] && arg[i] != '=')
	{
		if (arg[i] != str[i])
			return (0);
		i++;
	}
	if (arg[i] && arg[i] != '=')
		return (0);
	if (arg[i] == '=')
		*j = 2 + i;
	else
		*j = 1 + i;
	return (1);
}

int	main(int argc, char **argv, char **envp)
{
	t_minishell	shell;
	char		**args;
	int			fd_out;
	int			status;
	size_t		i;

	(void) argc;
	if (init_minishell(&shell, envp))
	{
		ft_putendl_fd("minishell: malloc: allocation failed", 2);
		return (1);
	}
	args = &argv[3];
	fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd_out <= 0)
	{
		ft_putendl_fd("minishell: open: cannot open file defaulting to stdout", 2);
		fd_out = 1;
	}
	if (parse_flag(argv[1], "export", &i))
		status = ft_export(&shell, args, fd_out);
	else if (parse_flag(argv[1], "unset", &i))
		status = ft_unset(&shell, args, fd_out);
	else if (parse_flag(argv[1], "echo", &i))
		status = ft_echo(&shell, args, fd_out);
	else if (parse_flag(argv[1], "cd", &i))
		status = ft_cd(&shell, args, fd_out);
	else if (parse_flag(argv[1], "pwd", &i))
		status = ft_pwd(&shell, args, fd_out);
	else if (parse_flag(argv[1], "env", &i))
		status = ft_env(&shell, args, fd_out);
	else
	{
		ft_putendl_fd("minishell: command not found", 2);
		close(fd_out);
		robin_free(shell.env);
		return (1);
	}
	close(fd_out);
	robin_free(shell.env);
	//main_loop(minishell);
	return (status);
}

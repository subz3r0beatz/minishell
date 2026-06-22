/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_basics.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/20 21:23:53 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/20 21:46:55 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

static int	insert_variable(t_robin *env, char *key, char *value)
{

}

static int	handle_pwd(t_robin *env)
{
	char			*pwd_value;
	char			*pwd;
	char			pwd_buf[8192];
	t_robin_node	robin_node;

	if (!getcwd(pwd_buf, 8192))
	{
		ft_putstr_fd("minishell: pwd: error retrieving current directory: ", 2);
		ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
		ft_putendl_fd(strerror(errno), 2);
		return (1);
	}
	pwd = ft_strdup("PWD");
	if (!pwd)
		return (1);
	pwd_value = ft_strdup(pwd_buf);
	if (!pwd)
		free(pwd_value);
	robin_node = create_node(env, pwd, pwd_value, 1);
	if (!robin_node)
		return (1);
	robin_insert(env, robin_node);
	free(pwd);
	free(pwd_value);
	return (0);
}

static int	handle_oldpwd(t_robin *env)
{
	if (handle_pwd(env))
		return (1);
	return (0);
}

int	create_basics(t_robin *env)
{
	t_robin_node	robin_node;


	robin_node = robin_search
	robin_node = create_node(env, "PWD", getcwd(NULL, 0), 0);
	if (!robin_node.value)
		return (0);
	robin_insert(env, robin_node);
	robin_node = create_node(env, "OLDPWD", getcwd(NULL, 0), 0);
	if (!robin_node.value)
		return (0);
	robin_insert(env, robin_node);
	return (1);
}

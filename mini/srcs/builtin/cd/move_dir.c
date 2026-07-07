#include "minishell.h"

static char	*get_pwd(t_minishell *shell)
{
	char	*pwd;
	char	buf[PATH_MAX];

	pwd = NULL;
	if (get_var_value(shell->env, "PWD", &pwd) || !pwd || pwd[0] != '/')
	{
		if (!getcwd(buf, PATH_MAX))
		{
			perror("minishell: cd: error retrieving current directory: "
				"getcwd: cannot access parent directories");
			return (NULL);
		}
		pwd = ft_strdup(buf);
	}
	else
		pwd = ft_strdup(pwd);
	if (!pwd)
		ft_putstr_fd("minishell: cd: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
	return (pwd);
}

static char	*parse_target(char *pwd, char *dir, int logical)
{
	char	*target;

	if (logical)
		target = canonalize_path(pwd, dir);
	else
		target = ft_strdup(dir);
	free(pwd);
	if (!target)
	{
		ft_putstr_fd("minishell: cd: malloc: "
			"cannot allocate memory\n", STDERR_FILENO);
		return (NULL);
	}
	if (!target[0])
	{
		free(target);
		target = ft_strdup(".");
		if (!target)
			ft_putstr_fd("minishell: cd: malloc: "
				"cannot allocate memory\n", STDERR_FILENO);
	}
	return (target);
}

static int	do_chdir(char *target, char *dir, int *logical)
{
	if (chdir(target))
	{
		if (*logical && !chdir(dir))
			*logical = 0;
		else
		{
			ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
			perror(dir);
			return (1);
		}
	}
	return (0);
}

static int	check_getcwd(char	**target, int logical, int e_flag)
{
	char	buf[PATH_MAX];

	if (!logical)
	{
		if (!getcwd(buf, PATH_MAX))
		{
			perror("minishell: cd: error retrieving current directory: "
				"getcwd: cannot access parent directories");
			if (e_flag)
				return (1);
		}
		else
		{
			free(*target);
			*target = ft_strdup(buf);
			if (!*target)
			{
				ft_putstr_fd("minishell: cd: malloc: "
					"cannot allocate memory\n", STDERR_FILENO);
				return (1);
			}
		}
	}
	return (0);
}

int	move_dir(t_minishell *shell, char **dir, int	logical, int e_flag)
{
	char	*pwd;
	char	*target;

	pwd = get_pwd(shell);
	if (!pwd)
		return (1);
	target = parse_target(pwd, *dir, logical);
	if (!target)
		return (1);
	if (do_chdir(target, *dir, &logical))
	{
		free(target);
		return (1);
	}
	if (check_getcwd(&target, logical, e_flag))
	{
		free(target);
		return (1);
	}
	free(*dir);
	*dir = target;
	return (0);
}

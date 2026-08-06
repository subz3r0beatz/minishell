#include "minishell.h"

static int	malloc_error(char *ptr1, char *ptr2)
{
	free(ptr1);
	free(ptr2);
	ft_putstr_fd("minishell: cd: malloc: "
		"cannot allocate memory\n", STDERR_FILENO);
	return (1);
}

static int	pwd_exists(t_minishell *shell, char *dir, char *pwd_value)
{
	char	*oldpwd_value;

	oldpwd_value = ft_strdup(pwd_value);
	if (!oldpwd_value)
		return (malloc_error(dir, NULL));
	if (update_var_value(shell, "OLDPWD", oldpwd_value))
	{
		if (insert_new_node(shell, "OLDPWD", oldpwd_value, 0))
			return (malloc_error(dir, oldpwd_value));
		free(oldpwd_value);
	}
	update_var_value(shell, "PWD", dir);
	return (0);
}

int	update_vars(t_minishell *shell, char *dir)
{
	char	*pwd_value;

	shell->double_root = (dir[0] == '/' && dir[1] == '/' && dir[2] != '/');
	ft_free_matrix(shell->exported, ft_memlen(shell->exported, sizeof(char *)));
	if (get_var_value(shell, "PWD", &pwd_value))
	{
		if (insert_new_node(shell, "PWD", dir, 0))
			return (malloc_error(dir, NULL));
		free(dir);
		update_var_value(shell, "OLDPWD", NULL);
	}
	else
		return (pwd_exists(shell, dir, pwd_value));
	return (0);
}

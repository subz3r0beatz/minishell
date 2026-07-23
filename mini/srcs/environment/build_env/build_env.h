/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_env.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 12:12:39 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/23 18:28:35 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILD_ENV_H
# define BUILD_ENV_H

typedef struct s_minishell	t_minishell;

int	build_env(t_minishell *shell, char **envp, char *argv0);
int	handle_oldpwd(t_minishell *shell);
int	handle_pwd(t_minishell *shell);
int	handle_shlvl(t_minishell *shell);
int	handle_underscore(t_minishell *shell, char *argv0);

#endif

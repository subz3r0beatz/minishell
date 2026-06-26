/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_env.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 12:12:39 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/26 12:14:07 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILD_ENV_H
# define BUILD_ENV_H

t_robin	*build_env(char **envp);
int		handle_oldpwd(t_robin *env);
int		handle_pwd(t_robin *env);
int		handle_shlvl(t_robin *env);
int		handle_underscore(t_robin *env);

#endif

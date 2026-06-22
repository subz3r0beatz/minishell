/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hash_functions.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 15:16:44 by fldumas-          #+#    #+#             */
/*   Updated: 2026/06/11 17:07:06 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HASH_FUNCTIONS_H
# define HASH_FUNCTIONS_H

size_t	djb2(const void *key);
size_t	fnv1a(const void *key);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 05:24:27 by fldumas-          #+#    #+#             */
/*   Updated: 2025/10/25 06:09:29 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_putnbrprint_fd(int n, int fd)
{
	if (n > 0)
	{
		ft_putnbrprint_fd(n / 10, fd);
		n = n % 10 + 48;
		write(fd, &n, 1);
	}
	return ;
}

void	ft_putnbr_fd(int n, int fd)
{
	if (n == 0)
		write(fd, "0", 1);
	else
	{
		if (n < 0)
		{
			if (n == -2147483648)
				write(fd, "-2147483648", 11);
			else
			{
				write(fd, "-", 1);
				n *= -1;
			}
		}
		ft_putnbrprint_fd(n, fd);
	}
	return ;
}

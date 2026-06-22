/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/18 00:45:19 by fldumas-          #+#    #+#             */
/*   Updated: 2025/10/25 06:41:42 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	ft_getorder(int n)
{
	int	order;

	order = 0;
	while (n)
	{
		order++;
		n /= 10;
	}
	return (order);
}

static void	ft_convertnb(char *str, int n, int order)
{
	if (n < 0)
	{
		str[0] = '-';
		n *= -1;
		order++;
	}
	str[order] = 0;
	while (order && n)
	{
		order--;
		str[order] = n % 10 + 48;
		n /= 10;
	}
}

char	*ft_itoa(int n)
{
	char	*str;
	int		order;

	if (n == 0)
	{
		str = ft_strdup("0");
		return (str);
	}
	else if (n == -2147483648)
	{
		str = ft_strdup("-2147483648");
		return (str);
	}
	order = ft_getorder(n);
	if (n < 0)
		str = (char *)malloc(sizeof(char) * (order + 2));
	else
		str = (char *)malloc(sizeof(char) * (order + 1));
	if (!str)
		return (NULL);
	ft_convertnb(str, n, order);
	return (str);
}

// int main(void)
// {
// 	ft_itoa(1000034);
// 	return 0;
// }

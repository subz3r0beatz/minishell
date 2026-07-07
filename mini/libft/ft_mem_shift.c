#include "libft.h"

void	ft_mem_shift(void *ptr, size_t count, size_t size, int dir)
{
	unsigned char *mem;
	size_t				len;

	if (!ptr || !count || !size
		|| (count > (size_t)-1 / size) || (dir != -1 && dir != 1))
		return ;
	mem = (unsigned char *)ptr;
	len = ft_memlen(ptr, size);
	if (len == 0)
		return ;
	if (count >= len)
	{
		ft_bzero(mem, len * size);
		return ;
	}
	if (dir == -1)
	{
		ft_memmove(mem, &mem[count * size], (len - count) * size);
		ft_bzero(&mem[(len - count) * size], count * size);
	}
	else
	{
		ft_memmove(&mem[count * size], mem, (len - count) * size);
		ft_bzero(mem, count * size);
	}
}

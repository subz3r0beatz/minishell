#include "libft.h"

char	*ft_join_split_prefix(const char **s, const char *del)
{
	char		*str;
	size_t	i;
	size_t	elements;
	size_t	len;
	size_t	del_len;

	elements = ft_memlen(s, sizeof(char *));
	del_len	= ft_strlen(del);
	i = 0;
	len = elements * del_len;
	while (s[i])
		len += ft_strlen(s[i++]);
	str = malloc(len * sizeof(char) + 1);
	if (!str)
		return (NULL);
	i = 0;
	while (s[i])
	{
		ft_strlcat(str, del, len);
		ft_strlcat(str, s[i], len);
		i++;
	}
	return (str);
}

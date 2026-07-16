#include "libft.h"

static size_t	count_words(char *str, char *delim, size_t delim_len)
{
	size_t	i;
	size_t	count;
	char	quote_state;

	i = 0;
	count = 0;
	quote_state = 0;
	while (str[i])
	{
		while (str[i] && !ft_strncmp(&str[i], delim, delim_len))
			i += delim_len;
		if (!str[i])
			break ;
		count++;
		while (str[i] && (quote_state || ft_strncmp(&str[i], delim, delim_len)))
		{
			if (quote_state == 0 && (str[i] == '\'' || str[i] == '"'))
				quote_state = str[i];
			else if (quote_state && str[i] == quote_state)
				quote_state = 0;
			i++;
		}
	}
	return (count);
}

static size_t	get_world_len(char *str, char *delim, size_t delim_len)
{
	size_t	len;
	char	quote_state;
	
	len = 0;
	quote_state = 0;
	while (str[len] && (quote_state || ft_strncmp(&str[len], delim, delim_len)))
	{
		if (quote_state == 0 && (str[len] == '\'' || str[len] == '"'))
			quote_state = str[len];
		else if (quote_state && str[len] == quote_state)
			quote_state = 0;
		len++;
	}
	return (len);
}

static void	copy_word(char *dst, char *src, size_t word_len)
{
	size_t	i;
	size_t	j;
	char	quote_state;

	i = 0;
	j = 0;
	quote_state = 0;
	while (i < word_len)
	{
		if (quote_state == 0 && (src[i] == '\'' || src[i] == '"'))
			quote_state = src[i];
		else if (quote_state && src[i] == quote_state)
			quote_state = 0;
		else
			dst[j++] = src[i];
		i++;
	}
	dst[j] = '\0';
}

static char	**get_splits(char **split, char *str, char *delim, size_t delim_len)
{
	size_t	i;
	size_t	count;
	size_t	world_len;
	
	i = 0;
	count = 0;
	while (str[i])
	{
		while (str[i] && !ft_strncmp(&str[i], delim, delim_len))
			i += delim_len;
		if (!str[i])
			break ;
		world_len = get_world_len(&str[i], delim, delim_len);
		split[count] = malloc((world_len + 1) * sizeof(char));
		if (!split[count])
		{
			ft_free_matrix(split, count);
			return (NULL);
		}
		copy_word(split[count], &str[i], world_len + 1);
		count++;
		i += world_len;
	}
	split[count] = NULL;
	return (split);
}

char	**ft_split_quotes_exl(char *str, char *delim)
{
	char	**split;
	size_t	delim_len;
	size_t	word_count;

	if (!str || !delim || !delim[0]
		|| ft_strchr(delim, '"') || ft_strchr(delim, '\''))
		return (NULL);
	delim_len = ft_strlen(delim);
	word_count = count_words(str, delim, delim_len);
	split = malloc((word_count + 1) * sizeof(char *));
	if (!split)
		return (NULL);
	split = get_splits(split, str, delim, delim_len);
	return (split);
}

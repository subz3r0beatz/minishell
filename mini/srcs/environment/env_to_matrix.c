#include "minishell.h"

static int  count_exported(void *key, void *value, void *args)
{

  return (0);
}

char  **env_to_matrix(t_minishell minishell)
{
  size_t  i;
  t_robin *env;

  robin_iter();
}

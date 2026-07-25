#ifndef EXPAND_H
# define EXPAND_H

int		expand(t_minishell *shell, t_ast_node *node);
char	*handle_special_param(t_minishell *shell, char *word, size_t *i);
char	*expand_home(t_minishell *shell, char *word);

#endif

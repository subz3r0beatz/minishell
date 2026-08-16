#ifndef REDIRECTIONS_H
# define REDIRECTIONS_H

int	redirections(t_redir *redir);
int	restore_fds(int in, int out);
int	handle_herestring(char *file);
int	handle_heredoc(t_minishell *shell, char *file, int *sigint_status);
int	collect_heredocs(t_minishell *shell, t_ast_node *node);

#endif

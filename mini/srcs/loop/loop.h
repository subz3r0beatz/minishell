#ifndef LOOP_H
# define LOOP_H

void	loop(t_minishell *shell);
char	*read_complete_input(t_minishell *shell, char *raw_input);
char	*read_line_non_interactive(int fd, char **stash);

#endif

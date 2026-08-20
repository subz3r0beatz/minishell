# Target binary name
NAME        = minishell

# Compiler and flags
CC          = cc
CFLAGS      = -Wall -Wextra -Werror
INCLUDES    = -Ilibft -Isrcs -Isrcs/builtin -Isrcs/builtin/cd -Isrcs/builtin/env -Isrcs/builtin/env/exec_env -Isrcs/builtin/env/parse_env -Isrcs/builtin/export -Isrcs/environment -Isrcs/environment/build_env -Isrcs/exec -Isrcs/exec/redirections -Isrcs/expand -Isrcs/lexer -Isrcs/lookup_table -Isrcs/loop -Isrcs/parser -Isrcs/parser/ast -Isrcs/prompt -Isrcs/robin_hash -Isrcs/robin_hash/hash_functions -Isrcs/signals
LDLIBS      = -lreadline

# All source files explicitly named
SRCS        = \
              libft/ft_atoi.c \
              libft/ft_bzero.c \
              libft/ft_calloc.c \
              libft/ft_char_split_quotes_exl.c \
              libft/ft_char_split_quotes_inc.c \
              libft/ft_dup_matrix.c \
              libft/ft_free_matrix.c \
              libft/ft_gnl.c \
              libft/ft_isalnum.c \
              libft/ft_isalpha.c \
              libft/ft_isascii.c \
              libft/ft_isdigit.c \
              libft/ft_isprint.c \
              libft/ft_iswhite.c \
              libft/ft_itoa.c \
              libft/ft_join_split_prefix.c \
              libft/ft_lstadd_back_bonus.c \
              libft/ft_lstadd_front_bonus.c \
              libft/ft_lstclear_bonus.c \
              libft/ft_lstdelone_bonus.c \
              libft/ft_lstiter_bonus.c \
              libft/ft_lstlast_bonus.c \
              libft/ft_lstmap_bonus.c \
              libft/ft_lstnew_bonus.c \
              libft/ft_lstsize_bonus.c \
              libft/ft_memchr.c \
              libft/ft_memcmp.c \
              libft/ft_memcpy.c \
              libft/ft_memlen.c \
              libft/ft_memmove.c \
              libft/ft_memset.c \
              libft/ft_mem_shift.c \
              libft/ft_putchar_fd.c \
              libft/ft_putendl_fd.c \
              libft/ft_putnbr_fd.c \
              libft/ft_putstr_fd.c \
              libft/ft_realloc.c \
              libft/ft_split.c \
              libft/ft_strchr.c \
              libft/ft_strcmp.c \
              libft/ft_strdup.c \
              libft/ft_striteri.c \
              libft/ft_strjoin_3.c \
              libft/ft_strjoin.c \
              libft/ft_strlcat.c \
              libft/ft_strlcpy.c \
              libft/ft_strlen.c \
              libft/ft_strmapi.c \
              libft/ft_strncmp.c \
              libft/ft_strnstr.c \
              libft/ft_strrchr.c \
              libft/ft_strtrim.c \
              libft/ft_substr.c \
              libft/ft_swap.c \
              libft/ft_tolower.c \
              libft/ft_toupper.c \
              srcs/builtin/cd/canonalize_path.c \
              srcs/builtin/cd/ft_cd.c \
              srcs/builtin/cd/move_dir.c \
              srcs/builtin/cd/parse_dir.c \
              srcs/builtin/cd/update_vars.c \
              srcs/builtin/env/exec_env/exec_env.c \
              srcs/builtin/env/exec_env/resolve_cmd_path.c \
              srcs/builtin/env/exit_env.c \
              srcs/builtin/env/ft_env.c \
              srcs/builtin/env/parse_env/add_variables.c \
              srcs/builtin/env/parse_env/handle_long_split.c \
              srcs/builtin/env/parse_env/handle_long_unset.c \
              srcs/builtin/env/parse_env/handle_split.c \
              srcs/builtin/env/parse_env/handle_unset.c \
              srcs/builtin/env/parse_env/parse_argv0.c \
              srcs/builtin/env/parse_env/parse_chdir_path.c \
              srcs/builtin/env/parse_env/parse_env_flags.c \
              srcs/builtin/env/parse_env/parse_long_argv0.c \
              srcs/builtin/env/parse_env/parse_long_chdir_path.c \
              srcs/builtin/env/parse_env/parse_long_flags.c \
              srcs/builtin/env/parse_env/parse_short_flags.c \
              srcs/builtin/env/parse_env/print_env_help.c \
              srcs/builtin/env/print_env.c \
              srcs/builtin/export/ft_export.c \
              srcs/builtin/export/parse_export_vars.c \
              srcs/builtin/export/print_export.c \
              srcs/builtin/export/unexport_vars.c \
              srcs/builtin/ft_echo.c \
              srcs/builtin/ft_exit.c \
              srcs/builtin/ft_pwd.c \
              srcs/builtin/ft_unset.c \
              srcs/environment/build_env/build_env.c \
              srcs/environment/build_env/handle_oldpwd.c \
              srcs/environment/build_env/handle_pid.c \
              srcs/environment/build_env/handle_pwd.c \
              srcs/environment/build_env/handle_shlvl.c \
              srcs/environment/build_env/handle_underscore.c \
              srcs/environment/check_exported.c \
              srcs/environment/create_env_node.c \
              srcs/environment/create_node.c \
              srcs/environment/delete_node.c \
              srcs/environment/env_to_matrix.c \
              srcs/environment/exit_shell.c \
              srcs/environment/free_shell.c \
              srcs/environment/get_var_value.c \
              srcs/environment/insert_new_node.c \
              srcs/environment/update_var_value.c \
              srcs/exec/clean_quotes.c \
              srcs/exec/exec_and.c \
              srcs/exec/exec_backgr.c \
              srcs/exec/exec_binary.c \
              srcs/exec/exec_builtin.c \
              srcs/exec/exec.c \
              srcs/exec/exec_cmd.c \
              srcs/exec/exec_newline.c \
              srcs/exec/exec_or.c \
              srcs/exec/exec_pipe.c \
              srcs/exec/exec_semi.c \
              srcs/exec/exec_subshell.c \
              srcs/exec/get_path.c \
              srcs/exec/redirections/handle_heredoc.c \
              srcs/exec/redirections/handle_herestring.c \
              srcs/exec/redirections/heredoc_read_loop.c \
              srcs/exec/redirections/redirections.c \
              srcs/exec/redirections/restore_fds.c \
              srcs/expand/expand.c \
              srcs/expand/expand_home.c \
              srcs/expand/expand_redirs.c \
              srcs/expand/expand_special_param.c \
              srcs/expand/expand_word.c \
              srcs/lexer/free_tokens.c \
              srcs/lexer/get_word_token_len.c \
              srcs/lexer/lexer.c \
              srcs/lookup_table/init_builtin_func_table.c \
              srcs/lookup_table/init_escape_table.c \
              srcs/lookup_table/init_exec_func_table.c \
              srcs/lookup_table/init_token_type_table.c \
              srcs/loop/extract_line.c \
              srcs/loop/loop.c \
              srcs/main.c \
              srcs/parser/ast/free_ast.c \
              srcs/parser/ast/new_cmd_node.c \
              srcs/parser/ast/new_op_node.c \
              srcs/parser/check_unclosed_quotes.c \
              srcs/parser/parse_cmd.c \
              srcs/parser/parse_list.c \
              srcs/parser/parse_logic.c \
              srcs/parser/parse_pipeline.c \
              srcs/parser/parser.c \
              srcs/parser/parse_redir.c \
              srcs/parser/parse_unclosed_quotes.c \
              srcs/parser/syntax_error.c \
              srcs/prompt/build_prompt.c \
              srcs/prompt/get_hostname.c \
              srcs/prompt/get_prompt_pwd.c \
              srcs/prompt/get_username.c \
              srcs/prompt/open_read.c \
              srcs/prompt/search_passwd.c \
              srcs/robin_hash/hash_functions/djb2.c \
              srcs/robin_hash/hash_functions/fnv1a.c \
              srcs/robin_hash/robin_expand.c \
              srcs/robin_hash/robin_free.c \
              srcs/robin_hash/robin_init.c \
              srcs/robin_hash/robin_insert.c \
              srcs/robin_hash/robin_iter.c \
              srcs/robin_hash/robin_remove.c \
              srcs/robin_hash/robin_search.c \
              srcs/signals/signals.c

# Object directory
OBJ_DIR     = build/

# Object files derived from source files, prefixed with the build directory
OBJS        = $(addprefix $(OBJ_DIR), $(SRCS:.c=.o))

# The 'all' rule must be the default one
all: $(NAME)

# Linking the final binary
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LDLIBS)

# Compiling source files into object files
$(OBJ_DIR)%.o: %.c
	# Create the target directory structure inside the build folder
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Valgrind rule for rigorous testing
valgrind: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes --track-origins=yes ./$(NAME)

# Clean rule for the build directory
clean:
	rm -rf $(OBJ_DIR)

# Full clean rule (objects + binary)
fclean: clean
	rm -f $(NAME)

# Rebuild rule
re: fclean all

.PHONY: all clean fclean re valgrind

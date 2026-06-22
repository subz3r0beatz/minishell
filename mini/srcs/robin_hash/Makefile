# ==============================================================================
#                                 VARIABLES
# ==============================================================================

NAME        = librobin.a

CC          = cc
CFLAGS      = -Wall -Wextra -Werror
RM          = rm -f
AR          = ar rcs

# Include directories (where your .h files are located)
INCLUDES    = -I. -I./hash_functions

# Source files (Notice: main.c is completely removed)
SRCS        = robin_init.c \
              robin_insert.c \
              robin_expand.c \
              robin_search.c \
              robin_remove.c \
              robin_free.c \
			  robin_iter.c \
			  robin_node_create.c \
			  hash_functions/djb2.c \
			  hash_functions/fnv1a.c

# Object files (automatically generated from SRCS)
OBJS        = $(SRCS:.c=.o)

# ==============================================================================
#                                  RULES
# ==============================================================================

# Default rule to build the library
all: $(NAME)

# Archive the object files into the static library
$(NAME): $(OBJS)
	$(AR) $(NAME) $(OBJS)

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Remove object files
clean:
	$(RM) $(OBJS)

# Remove object files AND the library
fclean: clean
	$(RM) $(NAME)

# Clean everything and recompile from scratch
re: fclean all

# Tell Make these aren't files, they are just commands
.PHONY: all clean fclean re

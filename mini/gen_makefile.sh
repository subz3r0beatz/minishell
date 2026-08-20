#!/bin/bash

# You can pass the project name as an argument, otherwise it defaults to "my_program"
NAME=${1:-my_program}
MAKEFILE="Makefile"

echo "Generating $MAKEFILE for project: $NAME..."

# 1. Find all unique directories containing .h files to generate -I flags
inc_dirs=$(find . -type f -name "*.h" -exec dirname {} \; | sort -u | sed 's|^\./||')
INCLUDES=""
for dir in $inc_dirs; do
    INCLUDES="$INCLUDES -I$dir"
done

# 2. Detect required libraries safely without breaking find's exit codes
LDLIBS=""

# Check for readline (matches <readline/...> or "readline/...")
if find . -type f \( -name "*.c" -o -name "*.h" \) -exec grep -E '^[[:space:]]*#[[:space:]]*include[[:space:]]*[<"]readline' {} + 2>/dev/null | grep -q .; then
    LDLIBS="$LDLIBS -lreadline"
fi

# Check for POSIX threads
if find . -type f \( -name "*.c" -o -name "*.h" \) -exec grep -E '^[[:space:]]*#[[:space:]]*include[[:space:]]*[<"]pthread\.h[>"]' {} + 2>/dev/null | grep -q .; then
    LDLIBS="$LDLIBS -pthread"
fi

# Check for math
if find . -type f \( -name "*.c" -o -name "*.h" \) -exec grep -E '^[[:space:]]*#[[:space:]]*include[[:space:]]*[<"]math\.h[>"]' {} + 2>/dev/null | grep -q .; then
    LDLIBS="$LDLIBS -lm"
fi

# Find any compiled local static libraries (e.g., libft.a)
local_libs=$(find . -type f -name "*.a" | sed 's|^\./||')
for lib in $local_libs; do
    LDLIBS="$LDLIBS $lib"
done

# 3. Write the header and variable declarations
cat <<EOF > $MAKEFILE
# Target binary name
NAME        = $NAME

# Compiler and flags
CC          = cc
CFLAGS      = -Wall -Wextra -Werror
INCLUDES    =$INCLUDES
LDLIBS      =$LDLIBS

# All source files explicitly named
SRCS        = \\
EOF

# 4. Find all .c files recursively, sort them, and append them explicitly
c_files=($(find . -type f -name "*.c" | sed 's|^\./||' | sort))

if [ ${#c_files[@]} -eq 0 ]; then
    echo "              # No .c files found!" >> $MAKEFILE
else
    for i in "${!c_files[@]}"; do
        if [ $i -eq $((${#c_files[@]} - 1)) ]; then
            echo "              ${c_files[$i]}" >> $MAKEFILE
        else
            echo "              ${c_files[$i]} \\" >> $MAKEFILE
        fi
    done
fi

# 5. Write the mandatory rules, build directory logic, and valgrind rule
cat <<'EOF' >> $MAKEFILE

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
EOF

echo "Done! Run 'make' to test it."

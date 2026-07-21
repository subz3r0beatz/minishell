TARGET = minishell
CC     = gcc
CFLAGS = -Wall -Wextra -Werror -fsanitize=undefined -g
OBJDIR = build

# 1. Identify independent sub-projects (like libft/)
REC_DIRS := $(dir $(wildcard */Makefile))

# 2. Function to recursively find all subdirectories deep inside a folder
#    (Excludes the build directory and recursive Makefile projects)
define get_subdirs
$(shell find $(1) -type d ! -path "./$(OBJDIR)*" $(foreach rec,$(REC_DIRS),! -path "./$(rec)*"))
endef

# Gather every single subdirectory inside your source folders
ALL_SRC_DIRS := $(call get_subdirs, .)

# 3. Gather all .c files anywhere in the project tree (excluding libft and build)
SRCS := $(foreach dir,$(ALL_SRC_DIRS),$(wildcard $(dir)/*.c))
OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

# 4. Include headers from every source directory found
CFLAGS += $(foreach dir,$(ALL_SRC_DIRS),-I$(dir))

# 5. Handle libft linking flags automatically
LDFLAGS := $(foreach dir,$(REC_DIRS),-L$(dir) -l$(subst /, ,$(subst lib,,$(dir))))

.PHONY: all clean fclean re $(REC_DIRS)

all: $(REC_DIRS) $(TARGET)

# Track A: Compile libft first
$(REC_DIRS):
	$(MAKE) -C $@

# Track B: Link absolutely everything together
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

# Compile files maintaining their nested directory structure inside build/
$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	@for dir in $(REC_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done

fclean: clean
	rm -f $(TARGET)
	@for dir in $(REC_DIRS); do \
		$(MAKE) -C $$dir fclean 2>/dev/null || $(MAKE) -C $$dir clean; \
	done

re: fclean all

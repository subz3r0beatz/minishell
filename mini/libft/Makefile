# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fldumas- <fldumas-@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/15 16:32:29 by fldumas-          #+#    #+#              #
#    Updated: 2025/10/25 22:27:55 by fldumas-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRC = \
ft_calloc.c \
ft_strlen.c \
ft_strlcpy.c \
ft_substr.c \
ft_tolower.c \
ft_toupper.c \
ft_strtrim.c \
ft_strrchr.c \
ft_strnstr.c \
ft_strnstr.c \
ft_strncmp.c \
ft_strlcpy.c \
ft_strlcat.c \
ft_strjoin.c \
ft_strdup.c \
ft_strchr.c \
ft_memset.c \
ft_memmove.c \
ft_memcpy.c \
ft_memcmp.c \
ft_memchr.c \
ft_isprint.c \
ft_isdigit.c \
ft_isascii.c \
ft_isalpha.c \
ft_isalnum.c \
ft_bzero.c \
ft_atoi.c \
ft_split.c \
ft_itoa.c \
ft_strmapi.c \
ft_striteri.c \
ft_putnbr_fd.c \
ft_putchar_fd.c \
ft_putstr_fd.c \
ft_putendl_fd.c

BONUS = \
ft_lstnew_bonus.c \
ft_lstadd_front_bonus.c \
ft_lstsize_bonus.c \
ft_lstlast_bonus.c \
ft_lstadd_back_bonus.c \
ft_lstdelone_bonus.c \
ft_lstclear_bonus.c \
ft_lstiter_bonus.c \
ft_lstmap_bonus.c

BONUSOBJ = ${BONUS:.c=.o}

NAME = libft.a

OBJ = ${SRC:.c=.o}

ALLOBJ = ${OBJ} ${BONUSOBJ}

COMP = cc
FLAGS = -Wall -Werror -Wextra

%.o : %.c
	${COMP} ${FLAGS} -c $< -o $@

all : ${NAME}

${NAME} : ${ALLOBJ}
	ar rcs $@ $^
	
bonus : ${BONUSOBJ}
	ar rcs ${NAME} ${BONUSOBJ}

clean : 
	rm -rf ${OBJ} ${BONUSOBJ}
fclean : clean
	rm -rf ${NAME}
re : fclean all

.PHONY : all clean bonus fclean re

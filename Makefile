# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/26 22:17:05 by rgilles           #+#    #+#              #
#    Updated: 2023/07/26 22:48:27 by rgilles          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	ft_ping

CC		=	clang
CFLAGS	=	-Wall -Wextra -Werror -g -I${LIBFT_D}${INCL} -I${INCL}

INCL	=	includes/
LIBFT_D	=	Libft/

SRCSD	=	srcs/
SRCSF	=	main.c
SRCS	=	$(addprefix ${SRCSD},${SRCSF})
OBJS	=	${SRCS:.c=.o}

all	:		${NAME}

${OBJS}:	${INCL}${NAME}.h ${LIBFT_D}${INCL}libft.h

${NAME}	:	${OBJS}
			make -C ${LIBFT_D}
			${CC} ${CFLAGS} -o ${NAME} ${OBJS} ${LIBFT_D}libft.a
clean	:
			find . -type f -name "*.o" -delete

fclean	:	clean
			find . -type f -name "*.a" -delete
			rm -rf ${NAME}

re	:		fclean all

.PHONY	:	all re clean fclean

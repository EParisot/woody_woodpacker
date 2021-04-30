# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: eparisot <eparisot@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#              #
#    Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	payload

ASM	=	payload.s
C   =	payload.c
C_OBJ   = $(C:.c=.o)

RM 		=	rm -rf

all		:	$(NAME)

$(NAME)	:	$(ASM) $(C) $(C_OBJ)
	nasm -f elf64 $(ASM) -o $(NAME).o
	ld $(NAME).o -o $(NAME)
	#ld $(C_OBJ) $(NAME).o -o $(NAME)
	#ld -shared -fstack-protector $(C_OBJ) -o $(NAME)

clean	:
	$(RM) $(NAME).o

fclean	: clean
	$(RM) $(NAME)

re		: clean all

.PHONY	: all re clean



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

NAME	=	sample

SRCS	=	sample.c

RM 		=	rm -rf

all		:	$(NAME)

$(NAME)	:	$(OBJS)
	clang -m64 $(SRCS) -o $(NAME)64

clean	:
	$(RM) $(NAME)64

re		: clean all

.PHONY	: all re clean



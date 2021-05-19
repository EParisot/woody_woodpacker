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

SRCS_ARGS	=	sample_args.c

RM 		=	rm -rf

all		:	$(NAME)

$(NAME)	:	$(SRCS) $(SRCS_ARGS)
	gcc -no-pie -m64 $(SRCS) -o $(NAME)64_no_pie
	gcc -no-pie -m64 $(SRCS_ARGS) -o $(NAME)_args_no_pie
	gcc -m64 $(SRCS) -o $(NAME)64
	gcc -m64 $(SRCS_ARGS) -o $(NAME)_args
	gcc -m64 -s $(SRCS) -o $(NAME)64_stripped
	gcc -m64 -s $(SRCS_ARGS) -o $(NAME)_args_stripped

clean	:
	$(RM) $(NAME)64

re		: clean all

.PHONY	: all re clean



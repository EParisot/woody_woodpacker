/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   woody_woodpacker.h                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WOODY_WOODPACKER_H

# define WOODY_WOODPACKER_H

# include "../libft/libft.h"
# include <unistd.h>
# include <stdlib.h>
#include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/mman.h>
#include <elf.h>

int				print_err(char *err, char *arg);
//void			set_cpu(u_int8_t cpu);
//uint64_t		cpu_64(uint64_t n);
//uint32_t		cpu_32(uint32_t n);

#endif
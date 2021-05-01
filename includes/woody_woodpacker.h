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
# include <elf.h>


#define PAYLOAD_SRC "payload/payload"

#define KEY "aaaabbbbccccdddd"
#define WORDSIZE 0x100000000


typedef struct s_env
{
	void 			*obj;
	void 			*obj_cpy;
	unsigned int	obj_size;
	unsigned int	obj_base;
	unsigned int	*payload_content;
	size_t 			payload_size;
	unsigned int	inject_offset;
	unsigned int    inject_addr;
	Elf64_Phdr 		*inject_phdr;
	Elf64_Shdr 		*inject_shdr;
	unsigned int 	entrypoint;
	unsigned int    page_offset;
	char			*text_addr;
	size_t 			text_size;
	int   			found_code_cave;
	u_int8_t 		cpu;
}				t_env;

int				print_err(char *err, char *arg);

uint64_t		cpu_64(uint64_t n, u_int8_t cpu);
uint32_t		cpu_32(uint32_t n, u_int8_t cpu);

void 			debug_dump(t_env *env, unsigned int *content, unsigned int start_addr, size_t size);
void 			debug_shdr(Elf64_Shdr shdr, char *label, const char *sh_strtab_p);
void 			debug_phdr(Elf64_Phdr phdr, char *label);

int 			build_payload(t_env *env);
int 			rabbit_encrypt(t_env *env, char *key);

#endif
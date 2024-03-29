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
# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <elf.h>


#define PAYLOAD_SRC "payload/payload"
#define WORDSIZE 0x100000000

typedef struct s_env
{
	void 			*obj;
	void 			*obj_cpy;
	unsigned int	*payload_content;
	char			*text_addr;
	size_t 			payload_size;
	size_t 			encrypt_size;
	size_t 			load_align;
	unsigned int	plt_offset;
	unsigned int	obj_size;
	unsigned int	new_obj_size;
	unsigned int	obj_base;
	unsigned int	inject_offset;
	unsigned int    inject_addr;
	unsigned int 	entrypoint;
	unsigned char	key[17];
	int   			found_code_cave;
	int   			found_code_cave_id;
	u_int8_t 		cpu;
}				t_env;

int				print_err(char *err, char *arg);

uint64_t		cpu_64(uint64_t n, u_int8_t cpu);
uint32_t		cpu_32(uint32_t n, u_int8_t cpu);

int 			check_corruption(void *obj, size_t size, char *obj_name);

void 			debug_dump(t_env *env, unsigned int *content, unsigned int start_addr, size_t size);
void 			debug_shdr(Elf64_Shdr shdr, char *label, const char *sh_strtab_p);
void 			debug_phdr(Elf64_Phdr phdr, char *label);

int 			build_payload(t_env *env);
int 			rabbit_encrypt(t_env *env);

int 			generate_key(t_env *env);
void 			print_key(t_env *env);

int 			dump_obj(t_env *env);
unsigned int 	replace_addr(t_env *env, unsigned int needle, unsigned int replace, int offset);

void 			clear_env(t_env *env);

#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   woody_woodpacker.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/woody_woodpacker.h"

static int dump_obj(t_env *env)
{
	int fd;

	if ((fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
	{
		printf("Error %d creating 'woody' file.\n", fd);
		return (-1);
	}
	write(fd, env->obj_cpy, env->new_size);
	close(fd);
	return (0);
}

static unsigned int replace_addr(t_env *env, unsigned int needle, unsigned int replace)
{
	size_t i = 0;
	int j = 0;

	for (i = 0; i < env->payload_size; ++i)
	{
		if (i * 8 < env->payload_size)
		{
			int found = 0;
			for (j = 0; j < 8; ++j)
			{
				if (i * 8 + j + 4 < env->payload_size && *(unsigned int *)(&((unsigned char *)(&((long unsigned int *)env->payload_content)[i]))[j]) == needle)
				{
					found = 1;
					break;
				}
			}
			if (found)
			{
				*(unsigned int *)(&((unsigned char *)(&((long unsigned int *)env->payload_content)[i]))[j]) = replace;
				break;

			}
		}
	}
	return (i * 8 + j);
}

static void inject_code(t_env *env)
{
	// replace entrypoint
	((Elf64_Ehdr *)env->obj_cpy)->e_entry = env->inject_addr; // new entrybpoint + base addr

	// replace start addr in payload (this is a negative offset)
	replace_addr(env, 0x39393939, - env->text_size + 0x24 - env->inject_dist - 0x70);
	// replace .text size in payload
	replace_addr(env, 0x40404040, (int)env->text_size);
	// replace key addr in payload
	replace_addr(env, 0x41414141, (*(long unsigned int*)env->key << 32) >> 32);
	replace_addr(env, 0x41414141, *(long unsigned int*)env->key >> 32);
	replace_addr(env, 0x41414141, (*(long unsigned int*)(env->key+8) << 32) >> 32);
	replace_addr(env, 0x41414141, (*(long unsigned int*)(env->key+8)) >> 32);
	
	// replace jmp addr in payload
	replace_addr(env, 0x42424242, - (env->text_size - 0x24) - env->inject_dist - (env->payload_size + 0x1d));

	// inject payload
	ft_memmove(env->obj_cpy + env->inject_offset, env->payload_content, env->payload_size);
}

static int parse_elf(t_env *env) 
{
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)env->obj_cpy;
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(env->obj_cpy + ehdr->e_shoff);
	// get pheader number
	int phnum = ehdr->e_phnum;
	// get first p_header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(env->obj_cpy + ehdr->e_phoff);
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = env->obj_cpy + sh_strtab->sh_offset;
	
	// get original entrypoint
	env->entrypoint = ehdr->e_entry;

	int load_found = 0;
	for (int i = 0; i < phnum; ++i)
	{
		// get base address
		if (phdr[i].p_type == PT_LOAD && load_found == 0)
		{
			env->obj_base = phdr[i].p_vaddr;
			load_found = 1;
		}
		// find code cave in executable segment or extend the padding
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5 && i + 1 < phnum)
		{
			if (phdr[i+1].p_offset - (phdr[i].p_offset + phdr[i].p_memsz) < env->payload_size)
			{
				// extend padding
				printf("Not enought place in PT_LOAD, extending PT_LOAD padding...\n");
			}
			
			env->inject_offset = phdr[i].p_offset + phdr[i].p_filesz;
			env->inject_addr = env->inject_offset + env->obj_base;
			env->inject_dist = env->obj_base + env->inject_offset - env->entrypoint - env->text_size - 0x10;

			ft_memmove(env->obj_cpy + phdr[i].p_offset + phdr[i].p_filesz + env->payload_size + env->payload_padd, 
						env->obj_cpy + phdr[i].p_offset + phdr[i].p_filesz + env->text_padd, 
						env->obj_size - (phdr[i].p_offset + phdr[i].p_filesz + env->text_padd));

			ft_bzero(env->obj_cpy + phdr[i].p_offset + phdr[i].p_filesz, env->payload_size + env->payload_padd);

			// offset every segment after PT_LOAD
			unsigned int added_size = (phdr[i].p_filesz + env->payload_size + env->payload_padd) - (phdr[i].p_filesz + env->text_padd);
			if (ehdr->e_shoff > phdr[i+1].p_offset)
			{
				ehdr->e_shoff += added_size;
				shdr = (Elf64_Shdr *)(env->obj_cpy + ehdr->e_shoff);
			}
			if (ehdr->e_phoff > phdr[i+1].p_offset)
			{
				ehdr->e_phoff += added_size;
				phdr = (Elf64_Phdr *)(env->obj_cpy + ehdr->e_phoff);
			}
			for (int k = 0; k < phnum; ++k)
			{
				if (phdr[k].p_offset >= phdr[i+1].p_offset)
				{
					phdr[k].p_offset += added_size;
					phdr[k].p_vaddr += added_size;
					phdr[k].p_paddr += added_size;
				}
			}
			for (int k = 0; k < shnum; ++k)
			{
				if (shdr[k].sh_offset >= phdr[i+1].p_offset)
				{
					shdr[k].sh_offset += added_size;
					shdr[k].sh_addr += added_size;
				}
			}

			// patch the pheader
			phdr[i].p_filesz += env->payload_size;
			phdr[i].p_memsz += env->payload_size;
			phdr[i].p_flags = PF_R | PF_W | PF_X;

			break;
		}
		// get .text section
		for (int i = 0; i < shnum; ++i)
		{
			if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".text"))
			{
				env->text_size = shdr[i].sh_size;
				env->text_addr = (char*)env->obj_cpy + shdr[i].sh_offset;
				env->text_offset = shdr[i].sh_offset + env->obj_base;
			}
		}
	}
	return 0;
}

static int generate_key(t_env *env)
{
	int fd = 0;

	if ((fd = open("/dev/urandom", O_RDONLY)) < 0)
		return 1;
	if (read(fd, env->key, 16) < 0)
		return 1;
	env->key[16] = 0;
	return 0;
}

static void print_key(t_env *env)
{
	printf("key_value:");
	for (size_t i = 0; i < 16; ++i)
	{
		printf("\\x%02x", env->key[i]);
	}
	printf("\n");
}

static int get_text_infos(t_env *env)
{
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)env->obj;
	// get sections number
	int phnum = ehdr->e_phnum;
	// get first section header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(env->obj + ehdr->e_phoff);

	for (int i = 0; i < phnum; ++i)
	{
		// get .text section
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5 && i + 1 < phnum)
		{
			env->p_size = phdr[i].p_filesz;
			while ((env->p_size + env->text_padd) % 0x1000)
			{
				++env->text_padd;
			}
			return 0;
		}
  	}
	return -1;
}

void get_payload_padd(t_env *env)
{
	while ((env->p_size + env->payload_size + env->payload_padd) % 0x1000)
	{
		++env->payload_padd;
	}
}

static int 		handle_obj(t_env *env)
{
	build_payload(env);
	get_text_infos(env);
	get_payload_padd(env);

	// copy original file
	env->new_size = env->obj_size + ((env->p_size + env->payload_size + env->payload_padd) - (env->p_size + env->text_padd));
	if ((env->obj_cpy = malloc(env->new_size)) == NULL)
	{
		printf("Error: can't duplicate file.\n");
		return 1;
	}
	ft_bzero(env->obj_cpy, env->new_size);
	ft_memcpy(env->obj_cpy, env->obj, env->obj_size);

	// get .text content
	if (parse_elf(env))
	{
		printf("Error parsing elf.\n");
		return 1;
	}

	printf("Original entrypoint: \t%08x\n", env->entrypoint);
	printf("Inserted entrypoint: \t%08x\n", env->inject_addr);
	printf("Inserted size: \t\t%08lx\n", env->payload_size);

	// generate Key
	if (generate_key(env))
	{
		printf("Error generating key.\n");
		return 1;
	}
	//ft_memcpy(env->key, "aaaabbbbccccdddd\0", 17);

	print_key(env);	
	
	inject_code(env);

	// encrypt .text
	/*if (rabbit_encrypt(env, env->key))						//ENCRYPT
	{
		printf("Error encrypting elf.\n");
		return 1;
	}*/
	
	// save new obj
	dump_obj(env);

	return 0;
}

static void 	clear_env(t_env *env)
{
	if (env->obj_cpy)
		free(env->obj_cpy);
	if (env->payload_content)
		free(env->payload_content);
	free(env);
}

static void 	woody_woodpacker(void *obj, size_t size, char *obj_name)
{
	char	hdr[6];
	t_env 	*env;

	ft_strncpy(hdr, obj, 6);
	if (hdr[0] == 0x7f && \
		hdr[1] == 'E' && \
		hdr[2] == 'L' && \
		hdr[3] == 'F' && \
		hdr[4] == ELFCLASS64)
	{
		// create env
		if ((env = (t_env *)malloc(sizeof(t_env))) == NULL)
		{
			return;
		}
		env->obj = obj;
		env->obj_cpy = NULL;
		env->obj_size = size;
		env->obj_base = 0;
		env->payload_content = NULL;
		env->payload_size = 0;
		env->payload_padd = 0;
		env->found_code_cave = 0;
		env->inject_dist = 0;
		env->text_size = 0;
		env->text_padd = 0;
		env->p_size = 0;
		env->text_addr = NULL;
		env->text_offset = 0;
		env->new_size = 0;
		env->entrypoint = 0;
		env->inject_offset = 0;
		env->inject_addr = 0;
		if (hdr[5] == 1)
			env->cpu = 1;
		else
			env->cpu = 0;
		handle_obj(env);
		clear_env(env);
	}
	else
	{
		printf("%s is not an ELF64. Exiting...\n", obj_name);
	}
}

static void			read_obj(char *obj_name)
{
	int				fd;
	void			*obj;
	size_t			size;

	if ((fd = open(obj_name, O_RDONLY)) < 0)
	{
		print_err("Error openning file", obj_name);
		return ;
	}
	if ((size = lseek(fd, (size_t)0, SEEK_END)) <= 0)
	{
		print_err("Error empty file", obj_name);
		close(fd);
		return ;
	}
	if ((obj = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0)) == \
			MAP_FAILED)
	{
		print_err("Error mapping file", obj_name);
		close(fd);
	}
	else
	{
		close(fd);
		woody_woodpacker(obj, size, obj_name);
		if (munmap(obj, size) < 0)
			print_err("Error munmap", "");
	}
}

int main(int argc, char **argv)
{
	if (argc == 2)
		read_obj(argv[1]);
	else if (argc == 1)
		read_obj("a.out\0");
	else {
		printf("Error: Too many args...\n");
	}
	return (0);
}
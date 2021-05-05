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
	write(fd, env->obj_cpy, (env->found_code_cave) ? env->obj_size : env->obj_size + env->payload_size + env->page_offset);
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
					//printf("FOUND\n");
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
	size_t payload_size =  env->payload_size;

	// replace entrypoint
	((Elf64_Ehdr *)env->obj_cpy)->e_entry = env->inject_addr; // new entrybpoint + base addr
	
	// replace start addr in payload
	replace_addr(env, 0x39393939, env->entrypoint);
	// replace end addr in payload
	replace_addr(env, 0x40404040, env->entrypoint + env->text_size);
	// replace key addr in payload
	replace_addr(env, 0x41414141, (*(long unsigned int*)env->key << 32) >> 32);
	replace_addr(env, 0x41414141, *(long unsigned int*)env->key >> 32);
	replace_addr(env, 0x41414141, (*(long unsigned int*)(env->key+8) << 32) >> 32);
	replace_addr(env, 0x41414141, (*(long unsigned int*)(env->key+8)) >> 32);
	
	// replace jmp addr in payload
	replace_addr(env, 0x42424242, - (env->text_size - 0x24) - (env->payload_size + 0x17));

	// inject payload
	ft_memmove(env->obj_cpy + env->inject_offset, env->payload_content, env->payload_size);
	
	// set the new injected phdr
	if (env->found_code_cave)
	{
		env->inject_phdr->p_filesz += payload_size;
		env->inject_phdr->p_memsz += payload_size;
		env->inject_phdr->p_flags = PF_R | PF_W | PF_X;
	}
	else
	{
		// set the .text header rights too
		env->text_phdr->p_flags = PF_R | PF_W | PF_X;
		// set the new injected phdr
		env->inject_phdr->p_type = PT_LOAD;
		env->inject_phdr->p_offset = env->inject_offset;
		env->inject_phdr->p_paddr = env->inject_addr;
		env->inject_phdr->p_vaddr = env->inject_addr;
		env->inject_phdr->p_filesz = payload_size;
		env->inject_phdr->p_memsz = payload_size;
		env->inject_phdr->p_flags = PF_R | PF_W | PF_X;
		env->inject_phdr->p_align = 0x1000;
		// set the new injected shdr
		env->inject_shdr->sh_type = SHT_PROGBITS;
		env->inject_shdr->sh_offset = env->inject_offset;
		env->inject_shdr->sh_addr = env->inject_addr;
		env->inject_shdr->sh_size = payload_size;
		env->inject_shdr->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
		env->inject_shdr->sh_addralign = 16;
	}
}

static unsigned int find_code_cave(unsigned char *start, unsigned int size, unsigned int code_size)
{
	unsigned int code_cave = 0;
	unsigned int best = 0;
	size_t i = 0;
	static size_t max;

	while(code_cave < size) 
	{
		i = 0;
		while(code_cave + i < size && start[code_cave + i] == 0)
		{
			++i;
		}
		if (i)
		{
			if (i > max && i > code_size)
			{
				best = code_cave;
				max = i;
			}
			code_cave += i;
		}
		else
		{
			++code_cave;
		}
	}
	return (best);
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
			//debug_phdr(phdr[i], "FIRST_PT_LOAD");
			env->obj_base = phdr[i].p_vaddr;
			load_found = 1;
		}
		// find code cave in executable segment
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5)
		{	
			if (i + 1 < phnum)
			{
				env->inject_offset = find_code_cave(env->obj_cpy + phdr[i].p_offset, phdr[i].p_align, env->payload_size);
				
				// DEBUG force file injection at end
				//env->inject_offset = 0;
				
				if (env->inject_offset != 0)
				{
					env->inject_offset += phdr[i].p_offset;
					env->inject_addr = env->inject_offset + env->obj_base;
					env->found_code_cave = 1;
					env->inject_phdr = &(phdr[i]);
				}
				else 
				{
					env->text_phdr = &(phdr[i]);
				}
			}
		}
		// get .note.* phdr if no code_cave found (so it will become the new injected phdr)
		if (phdr[i].p_type == PT_NOTE)
		{
			//debug_phdr(phdr[i], "PT_NOTE");
			if (env->found_code_cave == 0)
			{
				env->inject_phdr = &(phdr[i]);
				env->inject_offset = env->obj_size + env->page_offset;
				env->inject_addr = env->inject_offset + env->obj_base;
			}
			break;
		}
	}
	for (int i = 0; i < shnum; ++i)
	{
		// get .note.ABI-tag shdr if no code_cave found (so it will become the new injected phdr)
		if (shdr[i].sh_type == SHT_NOTE && ft_strequ(sh_strtab_p + shdr[i].sh_name, ".note.ABI-tag"))
		{
			//debug_shdr(shdr[i], "SHT_NOTE", sh_strtab_p);
			if (env->found_code_cave == 0)
			{
				env->inject_shdr = &(shdr[i]);
			}
			continue;
		}
		// get .text section
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".text"))
		{
			env->text_size = shdr[i].sh_size;
			env->text_addr = (char*)env->obj_cpy + shdr[i].sh_offset;
			env->text_offset = shdr[i].sh_offset + env->obj_base;
		}
  	}
	return 0;
}

static void get_page_offset(t_env *env)
{
	int i = 0;

	while ((env->obj_size + i) % 0x1000 != 0)
		++i;
	env->page_offset = i;
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

/*static void get_key(t_env *env)
{
	ft_memcpy(env->key, (char*)(env->obj_cpy + env->entrypoint - env->obj_base + 0x3e), 8);
	ft_memcpy(env->key + 8, (char*)(env->obj_cpy + env->entrypoint - env->obj_base + 0x48), 8);
	env->key[16] = 0;
}*/

static void print_key(t_env *env)
{
	printf("key_value:");
	for (size_t i = 0; i < 16; ++i)
	{
		printf("\\x%02x", env->key[i]);
	}
	printf("\n");
}

static int 		handle_obj(t_env *env)
{
	build_payload(env);

	get_page_offset(env);

	// copy original file
	if ((env->obj_cpy = malloc(env->obj_size + env->page_offset + env->payload_size)) == NULL)
	{
		printf("Error: can't duplicate file.\n");
		return 1;
	}
	ft_bzero(env->obj_cpy, env->obj_size + env->page_offset + env->payload_size);
	ft_memcpy(env->obj_cpy, env->obj, env->obj_size);
	
	// get .text content
	if (parse_elf(env))
	{
		printf("Error parsing elf.\n");
		return 1;
	}

	printf("Original entrypoint: \t%08x\n", env->entrypoint);
	printf("Inserted entrypoint: \t%08x\n", env->inject_addr);

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
		env->found_code_cave = 0;
		env->text_size = 0;
		env->text_addr = NULL;
		env->text_offset = 0;
		env->entrypoint = 0;
		env->inject_offset = 0;
		env->inject_addr = 0;
		env->page_offset = 0;
		env->inject_phdr = NULL;
		env->inject_shdr = NULL;
		env->text_phdr = NULL;
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
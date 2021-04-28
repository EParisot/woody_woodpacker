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
	write(fd, env->obj_cpy, (env->found_code_cave) ? env->obj_size : env->obj_size + env->payload_size + JUMP_SIZE);
	close(fd);
	return (0);
}

static void inject_code(t_env *env)
{
	//char pusha[] = {0x60, 0x0};
	//char popa[] = {0x61, 0x0};

	char mov[] = {0x48, 0xc7, 0xc0, 0x0};
	char jmp[] = {0xff, 0xe0, 0x0};
	unsigned int jmp_addr;

	size_t payload_size =  env->payload_size + ft_strlen(mov) + sizeof(unsigned int) + ft_strlen(jmp);
	//size_t payload_size = ft_strlen(pusha) + ft_strlen(mov) + sizeof(unsigned int) + ft_strlen(jmp) + ft_strlen(popa);

	// replace entrypoint
	((Elf64_Ehdr *)env->obj_cpy)->e_entry = env->inject_addr; // new entrybpoint + base addr

	//ft_memmove(env->obj_cpy + env->inject_offset, pusha, ft_strlen(pusha));

	// inject payload
	ft_memmove(env->obj_cpy + env->inject_offset, env->payload_content, env->payload_size);

	// inject code return mechanism
	ft_memmove(env->obj_cpy + env->inject_offset + env->payload_size, mov, ft_strlen(mov));
	jmp_addr = env->entrypoint; // jump back to original entrypoint
	ft_memmove(env->obj_cpy + env->inject_offset + env->payload_size + ft_strlen(mov), &jmp_addr, sizeof(unsigned int));
	ft_memmove(env->obj_cpy + env->inject_offset + env->payload_size + ft_strlen(mov) + sizeof(unsigned int), jmp, ft_strlen(jmp));
	
	//ft_memmove(env->obj_cpy + env->inject_offset + ft_strlen(mov) + sizeof(unsigned int) + ft_strlen(jmp), popa, ft_strlen(popa));
	
	// set the new injected phdr
	if (env->found_code_cave)
	{
		env->inject_phdr->p_filesz += payload_size;
		env->inject_phdr->p_memsz += payload_size;
	}
	else
	{
		env->inject_phdr->p_type = PT_LOAD;
		env->inject_phdr->p_offset = env->inject_offset;
		env->inject_phdr->p_paddr = env->inject_offset;
		env->inject_phdr->p_vaddr = env->inject_offset;
		env->inject_phdr->p_filesz = payload_size;
		env->inject_phdr->p_memsz = payload_size;
		env->inject_phdr->p_flags = PF_R | PF_X;
		env->inject_phdr->p_align = 0x1000;
		// set the new injected shdr
		env->inject_shdr->sh_type = SHT_PROGBITS;
		env->inject_shdr->sh_offset = env->inject_offset;
		env->inject_shdr->sh_addr = env->inject_offset;
		env->inject_shdr->sh_size = payload_size;
		env->inject_shdr->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
		env->inject_shdr->sh_addralign = 16;
	}
}

unsigned int find_code_cave(unsigned char *start, unsigned int end, unsigned int code_size) // TODO check payload size fits in cave
{
	unsigned int code_cave = 0;
	unsigned int best = 0;
	size_t i = 0;
	static size_t max;

	while(code_cave < end) 
	{
		i = 0;
		while(code_cave + i < end && start[code_cave + i] == 0)
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

int parse_elf(t_env *env) 
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
			env->obj_base += phdr[i].p_vaddr;
			load_found = 1;
		}
		// find code cave in executable segment
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5)
		{	
			if (i + 1 < phnum)
			{
				env->inject_offset = find_code_cave(env->obj_cpy + phdr[i].p_offset, phdr[i+1].p_offset, env->payload_size + JUMP_SIZE);
				if (env->inject_offset != 0)
				{
					env->inject_offset += phdr[i].p_offset;
					env->inject_addr = env->inject_offset + env->obj_base;
					env->found_code_cave = 1;
					env->inject_phdr = &(phdr[i]);
				}
				else
				{
					printf("No codecave found, appending to the end of file\n");
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
				env->inject_offset = env->obj_size;
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
			
			if ((env->text_content = malloc(env->text_size)) == NULL)
				return 1;
			ft_bzero(env->text_content, env->text_size);
			ft_memcpy(env->text_content, env->obj + shdr[i].sh_offset, env->text_size);
			/*// align sh size on 8
			int k = 0;
			while (env->text_size % 8 != 0)
				++k;
			debug_dump(env, env->text_content, env->entrypoint, env->text_size + k);*/
		}
  	}

	printf("\noriginal entrypoint: %08x\n", env->entrypoint);
	printf("new entrypoint: %08x\n", env->inject_addr);

	return 0;
}

static int 		handle_obj(t_env *env)
{
	build_payload(env);

	// copy original file
	if ((env->obj_cpy = malloc(env->obj_size + env->payload_size + JUMP_SIZE)) == NULL)
	{
		printf("Error: can't duplicate file.\n");
		return 1;
	}
	ft_bzero(env->obj_cpy, env->obj_size + env->payload_size + JUMP_SIZE);
	ft_memcpy(env->obj_cpy, env->obj, env->obj_size);
	
	// get .text content
	if (parse_elf(env))
	{
		printf("Error parsing elf.\n");
		return 1;
	}
		
	inject_code(env);

	printf("DEBUG injected code:");
	debug_dump(env, env->obj_cpy + env->inject_offset, env->inject_offset, env->payload_size + JUMP_SIZE);

	// TODO encrypt .text

	// save new obj
	dump_obj(env);
	return 0;
}

static void 	clear_env(t_env *env)
{
	if (env->obj_cpy)
		free(env->obj_cpy);
	if (env->text_content)
		free(env->text_content);
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
		printf("ELF64 detected in %s\n", obj_name);
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
		env->text_content = NULL;
		env->text_size = 0;
		env->entrypoint = 0;
		env->inject_offset = 0;
		env->inject_addr = 0;
		env->inject_phdr = NULL;
		env->inject_shdr = NULL;
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
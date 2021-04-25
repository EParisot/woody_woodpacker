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
	write(fd, env->obj_cpy, env->obj_size);
	close(fd);
	return (0);
}

void debug_dump(t_env *env)
{
	printf("\nDEBUG: .text\n.text size = %ld bytes", env->text_size);
	for (size_t j = 0; j * 4 < env->text_size; j += 1)
	{
		if (j % 4 == 0)
			printf("\n %04lx - ", env->entrypoint + j * 4);
		printf("%08x ", cpu_32(env->text_content[j], env->cpu));
	}
	printf("\n");
}

void debug_shdr(Elf64_Shdr shdr, char *label, const char *sh_strtab_p)
{
	printf("\nDEBUG: %s SHdr\n", label);
	printf("sh_name: %s\n", sh_strtab_p + shdr.sh_name);
	printf("sh_offset: %08lx\n", shdr.sh_offset);
	printf("sh_addr: %08lx\n", shdr.sh_addr);
	printf("sh_size: %08lx\n", shdr.sh_size);
	printf("sh_flags: %04lx\n", shdr.sh_flags);
	printf("sh_addralign: %02lx\n", shdr.sh_addralign);
}

void debug_phdr(Elf64_Phdr phdr, char *label)
{
	printf("\nDEBUG: %s PHdr\n", label);
	printf("p_offset: %08lx\n", phdr.p_offset);
	printf("p_vaddr: %08lx\n", phdr.p_vaddr);
	printf("p_filesz: %08lx\n", phdr.p_filesz);
	printf("p_memsz: %08lx\n", phdr.p_memsz);
	printf("p_flags: %04x\n", phdr.p_flags);
	printf("p_align: %02lx\n", phdr.p_align);
}

static void find_empty_space(t_env *env, Elf64_Phdr *phdr) // TODO FIND A BETTER PLACE
{
	static size_t done;
	//size_t curr_size = (phdr->p_vaddr + phdr->p_align) - (phdr->p_vaddr + phdr->p_filesz);

	if (done == 0 && phdr->p_flags == 5)
	{
		done = 1;
		env->inject_addr = phdr->p_vaddr + phdr->p_filesz;
		env->injected_hdr = phdr;
	}
}

static void inject_code(t_env *env)
{
	char code[] = "x31xc0x31xdbx31xd2x68x72x6cx64x21xc6x44x24x03x0ax68x6fx20x77x6fx68x48x65x6cx6cx89xe1xb2x0cxb0x04xb3x01xcdx80xb2x0cx01xd4x00";
	char jmp[] = "xe9xffxffxffxff";
	char pusha[] = "x60";
	char popa[] = "x61";
	unsigned int jmp_addr;

	// replace entrypoint
	((Elf64_Ehdr *)env->obj_cpy)->e_entry = env->inject_addr;
	// inject code
	ft_memmove(env->obj_cpy + env->inject_addr, pusha, ft_strlen(pusha));
	ft_memmove(env->obj_cpy + env->inject_addr + ft_strlen(pusha), code, ft_strlen(code));
	ft_memmove(env->obj_cpy + env->inject_addr + ft_strlen(pusha) + ft_strlen(code), popa, ft_strlen(popa));
	// set the pointer back to original entrypoint
	jmp_addr = env->entrypoint - (env->inject_addr + ft_strlen(code));
	ft_memmove(jmp + 1, &jmp_addr, 4);
	ft_memmove(env->obj_cpy + env->inject_addr + ft_strlen(pusha) + ft_strlen(code) + ft_strlen(popa), jmp, ft_strlen(jmp));
	// set the new injected hdr size
	((Elf64_Phdr *)env->injected_hdr)->p_filesz += ft_strlen(pusha) + ft_strlen(code) + ft_strlen(popa) + ft_strlen(jmp);
	((Elf64_Phdr *)env->injected_hdr)->p_memsz += ft_strlen(pusha) + ft_strlen(code) + ft_strlen(popa) + ft_strlen(jmp);

	 // TODO ADAPT TO BETTER PLACED CODE
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
	// get entrypoint
	env->entrypoint = ehdr->e_entry;

	for (int i = 0; i < phnum; ++i)
	{
		/*// get .note.* section
		if (phdr[i].p_type == PT_NOTE)
		{
			debug_phdr(phdr[i], "PT_NOTE");
		}
		// find best place to inject
		else*/ if (phdr[i].p_type == PT_LOAD) {
			find_empty_space(env, &phdr[i]);
			debug_phdr(phdr[i], "PT_LOAD");
		}
	}
	
	printf("\nPlace to inject = %08x\n", env->inject_addr);

	for (int i = 0; i < shnum; ++i)
	{
		/*// get .note.ABI-tag section
		if (shdr[i].sh_type == SHT_NOTE)
		{
			debug_shdr(shdr[i], "SHT_NOTE", sh_strtab_p);
		}*/
		// get .text section
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".text"))
		{
			env->text_size = shdr[i].sh_size;
			// align sh size on 8
			while (env->text_size % 8 != 0)
				++(env->text_size);
			if ((env->text_content = malloc(env->text_size)) == NULL)
				return 1;
			ft_bzero(env->text_content, env->text_size);
			ft_memcpy(env->text_content, env->obj + shdr[i].sh_offset, env->text_size);
			debug_dump(env);
		}
  	}

	inject_code(env);

	return 0;
}

static int 		handle_obj(t_env *env)
{
	// copy original file
	if ((env->obj_cpy = malloc(env->obj_size)) == NULL)
	{
		printf("Error: can't duplicate file.\n");
		return 1;
	}
	ft_memcpy(env->obj_cpy, env->obj, env->obj_size);
	// get .text content
	if (parse_elf(env))
	{
		printf("Error parsing elf.\n");
		return 1;
	}
	// TODO encrypt .text
	dump_obj(env);
	return 0;
}

static void 	clear_env(t_env *env)
{
	if (env->obj_cpy)
		free(env->obj_cpy);
	if (env->text_content)
		free(env->text_content);
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
		env->text_content = NULL;
		env->text_size = 0;
		env->entrypoint = 0;
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
		print_err("Error mapping file", obj_name);
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
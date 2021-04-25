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

	if ((fd = open("woody", O_WRONLY | O_CREAT)) < 0)
	{
		printf("Error %d creating 'woody' file.\n", fd);
		return (-1);
	}
	write(fd, env->obj, env->obj_size);
	close(fd);
	return (0);
}

void debug_dump(t_env *env)
{
	printf("text size = %ld bytes", env->text_size);
	for (size_t j = 0; j * 4 < env->text_size; j += 1)
	{
		if (j % 4 == 0)
			printf("\n %04lx - ", env->entrypoint + j * 4);
		printf("%08x ", cpu_32(env->text_content[j], env->cpu));
	}
	printf("\n");
}

int get_text(t_env *env) 
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)env->obj;
	int shnum = ehdr->e_shnum;
	Elf64_Shdr *shdr = (Elf64_Shdr *)(env->obj + ehdr->e_shoff);
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = env->obj + sh_strtab->sh_offset;

	for (int i = 0; i < shnum; ++i)
	{
		// get .text section
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".text"))
		{
			env->text_size = shdr[i].sh_size;
			// align sh size on 8
			while (env->text_size % 8 != 0)
				(env->text_size)++;
			if ((env->text_content = malloc(env->text_size)) == NULL)
				return 1;
			ft_bzero(env->text_content, env->text_size);
			ft_memcpy(env->text_content, env->obj + shdr[i].sh_offset, env->text_size);
			debug_dump(env);
			return 0;
		}
  	}
	return 1;
}

static int handle_obj(t_env *env)
{
	// copy original file
	if ((env->obj_cpy = malloc(env->obj_size)) == NULL)
	{
		printf("Error: can't duplicate file.\n");
		return 1;
	}
	ft_memcpy(env->obj_cpy, env->obj, env->obj_size);
	// get entrypoint

	// get .text content
	if (get_text(env))
	{
		printf("Error: copying text section.\n");
		return 1;
	}
	// TODO encrypt .text
	dump_obj(env);
	return 0;
}

static void clear_env(t_env *env)
{
	if (env->obj_cpy)
		free(env->obj_cpy);
	if (env->text_content)
		free(env->text_content);
	free(env);
}

static void 		woody_woodpacker(void *obj, size_t size, char *obj_name)
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
		if ((env = (t_env *)malloc(sizeof(t_env))) == NULL)
		{
			return;
		}
		env->obj = obj;
		env->obj_size = size;
		env->text_content = NULL;
		env->text_size = 0;
		env->entrypoint = 0;
		env->obj_cpy = NULL;
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
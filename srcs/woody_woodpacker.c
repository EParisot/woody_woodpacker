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

static int dump_obj(void *obj, size_t size)
{
	int fd;

	if ((fd = open("woody", O_WRONLY)) < 0)
	{
		printf("Error creating 'woody' file.\n");
		return (-1);
	}
	write(fd, obj, size);
	close(fd);
	return (0);
}

static void handle_obj(void *obj, size_t size)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)obj;
	int shnum = ehdr->e_shnum;
	Elf64_Shdr *shdr = (Elf64_Shdr *)(obj + ehdr->e_shoff);
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = obj + sh_strtab->sh_offset;
	
	for (int i = 0; i < shnum; ++i) {
    	printf("%2d: %4d '%s'\n", i, shdr[i].sh_name, sh_strtab_p + shdr[i].sh_name);
  	}
	// TODO corrupt file
	dump_obj(obj, size);
}

static void 		woody_woodpacker(void *obj, size_t size, char *obj_name)
{
	char	magic_nb[5];

	ft_strncpy(magic_nb, obj, 5);
	if (magic_nb[0] == 0x7f && \
		magic_nb[1] == 'E' && \
		magic_nb[2] == 'L' && \
		magic_nb[3] == 'F' && \
		magic_nb[4] == ELFCLASS64)
	{
		printf("ELF64 detected in %s\n", obj_name);
		handle_obj(obj, size);
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
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

static void handle_obj(void *obj)
{
	Elf64_Ehdr	*ehdr = (Elf64_Ehdr *)obj;
	Elf64_Shdr 	*shdr = (Elf64_Shdr *)(obj + ehdr->e_shoff);
	Elf64_Shdr 	*sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = obj + sh_strtab->sh_offset;
	int shnum = ehdr->e_shnum;

	for (int i = 0; i < shnum; ++i) {
    	printf("%2d: %4d '%s'\n", i, shdr[i].sh_name, sh_strtab_p + shdr[i].sh_name);
  	}
}

static void 		woody_woodpacker(void *obj, void *end, char *obj_name)
{
	char	magic_nb[5];
	(void)end;

	ft_strncpy(magic_nb, obj, 5);
	if (magic_nb[0] == 0x7f && \
		magic_nb[1] == 'E' && \
		magic_nb[2] == 'L' && \
		magic_nb[3] == 'F' && \
		magic_nb[4] == ELFCLASS64)
	{
		printf("ELF64 detected in %s\n", obj_name);
		handle_obj(obj);
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
	struct stat		buf;

	if ((fd = open(obj_name, O_RDONLY)) < 0)
	{
		print_err("Error openning file", obj_name);
		return ;
	}
	if (fstat(fd, &buf) < 0)
	{
		print_err("Error stating file", obj_name);
		close(fd);
		return ;
	}
	if ((obj = mmap(0, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == \
			MAP_FAILED)
		print_err("Error mapping file", obj_name);
	else
	{
		close(fd);
		woody_woodpacker(obj, obj + buf.st_size, obj_name);
		if (munmap(obj, buf.st_size) < 0)
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
		for (int i = 1; i < argc; ++i)
		{
			read_obj(argv[i]);
		}
	}
	return (0);
}
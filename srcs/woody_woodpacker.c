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
	void *obj_cpy;
	unsigned int *text_content;
	size_t text_size;
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)obj;
	int shnum = ehdr->e_shnum;
	Elf64_Shdr *shdr = (Elf64_Shdr *)(obj + ehdr->e_shoff);
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = obj + sh_strtab->sh_offset;
	
	for (int i = 0; i < shnum; ++i) {
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".text"))
		{
			text_size = shdr[i].sh_size;
			// align size on 8
			while (text_size % 8 != 0)
				text_size++;
			printf("text size = %ld\n", text_size);
			if ((text_content = malloc(text_size)) == NULL)
			{
				printf("Error copying text section.\n");
				return ;
			}
			ft_bzero(text_content, text_size);
			ft_memcpy(text_content, obj + shdr[i].sh_offset, text_size);
			for (size_t j = 0; j * 4 < text_size; j += 1)
			{
				if (j % 4 == 0)
					printf("\n %04lx - ", j);
				printf("%08x ", cpu_32(text_content[j]));
			}
			printf("\n");
			free(text_content);
		}
  	}
	if ((obj_cpy = malloc(size)) == NULL)
	{
		printf("Error: can't duplicate file...");
		return ;
	}
	ft_memcpy(obj_cpy, obj, size);
	// TODO corrupt file
	dump_obj(obj_cpy, size);
	free(obj_cpy);
}

static void 		woody_woodpacker(void *obj, size_t size, char *obj_name)
{
	char	hdr[6];

	ft_strncpy(hdr, obj, 6);
	if (hdr[0] == 0x7f && \
		hdr[1] == 'E' && \
		hdr[2] == 'L' && \
		hdr[3] == 'F' && \
		hdr[4] == ELFCLASS64)
	{
		printf("ELF64 detected in %s\n", obj_name);
		if (hdr[5] == 1)
			set_cpu(1);
		else
			set_cpu(0);
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
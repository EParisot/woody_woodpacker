/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/woody_woodpacker.h"

void 	clear_env(t_env *env)
{
	if (env->obj_cpy)
		free(env->obj_cpy);
	if (env->payload_content)
		free(env->payload_content);
	free(env);
}

int dump_obj(t_env *env)
{
	int fd;

	if ((fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
	{
		printf("Error %d creating 'woody' file.\n", fd);
		return (-1);
	}
	write(fd, env->obj_cpy, (env->found_code_cave) ? env->obj_size : env->page_offset + env->payload_size);
	close(fd);
	return (0);
}

unsigned int replace_addr(t_env *env, unsigned int needle, unsigned int replace)
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

int generate_key(t_env *env)
{
	int fd = 0;

	if ((fd = open("/dev/urandom", O_RDONLY)) < 0)
		return 1;
	if (read(fd, env->key, 16) < 0)
		return 1;
	env->key[16] = 0;
	return 0;
}

void print_key(t_env *env)
{
	printf("key_value: ");
	for (size_t i = 0; i < 16; ++i)
	{
		printf("%02x", env->key[i]);
	}
	printf("\n");
}

int				print_err(char *err, char *arg)
{
	ft_putstr(err);
	if (ft_strlen(arg))
	{
		ft_putstr(" '");
		ft_putstr(arg);
		ft_putstr("'");
	}
	ft_putchar('\n');
	return (1);
}

uint32_t		cpu_32(uint32_t n, uint8_t cpu)
{
	if (cpu != 0)
		return (ft_swap_32(n));
	return (n);
}

uint64_t		cpu_64(uint64_t n, uint8_t cpu)
{
	if (cpu != 0)
		return (ft_swap_64(n));
	return (n);
}

void debug_dump(t_env *env, unsigned int *content, unsigned int start_addr, size_t size)
{
	printf("\nDEBUG: size = %ld bytes", size);
	for (size_t j = 0; j * 4 < size; j += 1)
	{
		if (j % 4 == 0)
			printf("\n %04lx - ", start_addr + j * 4);
		printf("%08x ", cpu_32(content[j], env->cpu));
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
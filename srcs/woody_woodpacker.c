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

static void inject_code(t_env *env)
{
	// replace entrypoint
	((Elf64_Ehdr *)env->obj_cpy)->e_entry = env->inject_addr + env->plt_offset; // new entrybpoint + base addr

	// calc dist between original entry and the end and inject point
	unsigned int inject_dist = env->inject_addr - env->entrypoint;
	
	// replace start addr in payload (this is a negative offset)
	replace_addr(env, 0x39393939, -(inject_dist + 4), 1);
	// replace encrypt size in payload
	replace_addr(env, 0x40404040, env->encrypt_size, 0);
	// replace key addr in payload
	replace_addr(env, 0x41414141, (*(long unsigned int*)env->key << 32) >> 32, 0);
	replace_addr(env, 0x41414141, *(long unsigned int*)env->key >> 32, 0);
	replace_addr(env, 0x41414141, (*(long unsigned int*)(env->key+8) << 32) >> 32, 0);
	replace_addr(env, 0x41414141, (*(long unsigned int*)(env->key+8)) >> 32, 0);
	
	// replace jmp addr in payload
	replace_addr(env, 0x42424242, -inject_dist, 1);

	// inject payload
	ft_memmove(env->obj_cpy + env->inject_offset, env->payload_content, env->payload_size);
}

void find_injection_point(t_env *env) {
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)env->obj;
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(env->obj + ehdr->e_shoff);
	// get pheader number
	int phnum = ehdr->e_phnum;
	// get first p_header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(env->obj + ehdr->e_phoff);
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = env->obj + sh_strtab->sh_offset;
	// get original entrypoint
	env->entrypoint = ehdr->e_entry;
	long unsigned int bss_offset = 0;
	size_t size = 0;
	int load_found = 0;

	for (int i = 0; i < phnum; ++i) {
		// get base address
		if (load_found == 0 && phdr[i].p_type == PT_LOAD) {
			env->obj_base = phdr[i].p_vaddr;
			env->load_align = phdr[i].p_align;
			load_found = 1;
		}
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5 && i + 1 < phnum) {
			if (phdr[i+1].p_offset - (phdr[i].p_offset + phdr[i].p_memsz) > env->payload_size) {
				printf("Found code cave in PT_LOAD at %lx\n", phdr[i].p_offset);
				env->inject_offset = phdr[i].p_offset + phdr[i].p_memsz;
				env->inject_addr = env->obj_base + env->inject_offset;
				env->found_code_cave = 1;
				env->found_code_cave_id = i;
			}
			else {
				printf("Not enought space in PT_LOAD, injecting after last section.\n");
				
				// parse sections
				for (int i = 0; i < shnum; i++) {
					if (strcmp(sh_strtab_p + shdr[i].sh_name, ".bss") == 0) {
						bss_offset = shdr[i].sh_offset;
						size += shdr[i].sh_size;
						while (size % shdr[i].sh_addralign) {
							++size;
						}
					}
					if (shdr[i].sh_offset > bss_offset) {
						size += shdr[i].sh_offset;
						while (size % shdr[i].sh_addralign) {
							++size;
						}
					}
				}
				env->inject_offset = bss_offset + size;
				env->inject_addr = env->obj_base + env->inject_offset;
			}
		}
	}
	unsigned int fini = 0;
	for (int j = 0; j < shnum; ++j)
	{
		if (ft_strequ(sh_strtab_p + shdr[j].sh_name, ".fini"))
		{
			fini = shdr[j].sh_offset;
		}
	}
	// set the size to encrypt from entrypoint
	env->encrypt_size = fini - env->entrypoint;
}

void tweak_elf(t_env *env) {
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

	for (int i = 0; i < phnum; i++)
	{
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5) {
			// set the text address and set write rights to decrypt
			env->text_addr = (char*)env->obj_cpy + (env->entrypoint - env->obj_base);
			phdr[i].p_flags = PF_R | PF_W | PF_X;
			break;
		}
	}
	if (env->found_code_cave == 1) {
		// patch the pheader
		phdr[env->found_code_cave_id].p_filesz += env->payload_size;
		phdr[env->found_code_cave_id].p_memsz += env->payload_size;
		phdr[env->found_code_cave_id].p_flags = PF_R | PF_W | PF_X;
	}
	else {
		for (int i = 0; i < phnum; ++i) {
			// get .note.* phdr if no code_cave found (so it will become the new injected phdr)
			if (phdr[i].p_type == PT_NOTE) {
				printf("Found PT_NOTE at %lx, turning it to PT_LOAD.\n", phdr[i].p_offset);
				// set the new injected phdr
				phdr[i].p_type = PT_LOAD;
				phdr[i].p_offset = env->inject_offset;
				phdr[i].p_paddr = env->inject_addr;
				phdr[i].p_vaddr = env->inject_addr;
				phdr[i].p_filesz = env->payload_size;
				phdr[i].p_memsz = env->payload_size;
				phdr[i].p_flags = PF_R | PF_W | PF_X;
				phdr[i].p_align = env->load_align;
				break;
			}
		}
		for (int i = 0; i < shnum; ++i) {
			// get .note.ABI-tag shdr if no code_cave found (so it will become the new injected shdr)
			if (shdr[i].sh_type == SHT_NOTE && strcmp(sh_strtab_p + shdr[i].sh_name, ".note.ABI-tag") == 0) {
				printf("Found .note.ABI-tag at %lx, turning it to SHT_PROGBITS.\n", shdr[i].sh_offset);
				// set the new injected shdr
				shdr[i].sh_type = SHT_PROGBITS;
				shdr[i].sh_offset = env->inject_offset;
				shdr[i].sh_addr = env->inject_addr;
				shdr[i].sh_size = env->payload_size;
				shdr[i].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
				shdr[i].sh_addralign = 16;
				break;
			}
		}
	}
}


static int handle_obj(t_env *env)
{
	build_payload(env);
	// get injection location
	find_injection_point(env);
	if (env->inject_offset + env->payload_size > env->obj_size) {
		env->new_obj_size = env->inject_offset + env->payload_size;
	}
	// copy original file
	if ((env->obj_cpy = malloc(env->new_obj_size + 1)) == NULL) {
		printf("Error: can't duplicate file.\n");
		return -1;
	}
	ft_memset(env->obj_cpy, 0, env->new_obj_size + 1);
	ft_memcpy(env->obj_cpy, env->obj, env->obj_size);
	// tweak target file's headers
	tweak_elf(env);
	// generate Key
	if (generate_key(env)) {
		printf("Error generating key.\n");
		return -1;
	}
	//ft_memcpy(env->key, "aaaabbbbccccdddd\0", 17);
	print_key(env);	
	inject_code(env);
	//debug_dump(env, (unsigned int *)env->text_addr, env->entrypoint, env->encrypt_size);
	// encrypt .text
	if (rabbit_encrypt(env)) { // ENCRYPT
		printf("Error encrypting elf.\n");
		return -1;
	}
	//debug_dump(env, (unsigned int *)env->text_addr, env->entrypoint, env->encrypt_size);
	// save new obj
	if (dump_obj(env)) {
		printf("Error dumping new object.\n");
		return -1;
	}
	// inject and dump new obj
	printf("Original entrypoint: \t%08x\n", env->entrypoint);
	printf("Inserted entrypoint: \t%08x\n", env->inject_addr);
	printf("Inserted size: \t\t%08lx\n", env->payload_size);

	return 0;
}

static int woody_woodpacker(void *obj, size_t size)
{
	t_env 	*env;
	int ret = 0;

	// create env
	if ((env = (t_env *)malloc(sizeof(t_env))) == NULL) {
		printf("Error: can't allocate memory.\n");
		return -1;
	}
	env->obj = obj;
	env->obj_cpy = NULL;
	env->obj_size = size;
	env->new_obj_size = size;
	env->obj_base = 0;
	env->payload_content = NULL;
	env->payload_size = 0;
	env->plt_offset = 0;
	env->found_code_cave = 0;
	env->found_code_cave_id = 0;
	env->encrypt_size = 0;
	env->text_addr = NULL;
	env->load_align = 0;
	env->entrypoint = 0;
	env->inject_offset = 0;
	env->inject_addr = 0;

	if (((char*)obj)[5] == 1)
		env->cpu = 1;
	else
		env->cpu = 0;
	
	if (handle_obj(env)) {
		printf("Error handling object.\n");
		ret = -1;
	}
	clear_env(env);
	return ret;
}

static int	read_obj(char *obj_name)
{
	int	fd;
	void *obj;
	size_t size;
	int ret = 0;

	if ((fd = open(obj_name, O_RDONLY)) < 0) {
		print_err("Error openning file", obj_name);
		return -1;
	}
	if ((size = lseek(fd, (size_t)0, SEEK_END)) <= 0) {
		print_err("Error empty file", obj_name);
		close(fd);
		return -1;
	}
	if ((obj = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0)) == \
			MAP_FAILED)
	{
		print_err("Error mapping file", obj_name);
		close(fd);
		return -1;
	}
	else {
		close(fd);
		if (check_corruption(obj, size, obj_name) == 0) {
			if (woody_woodpacker(obj, size)) {
				ret = -1;
			}
		}
		if (munmap(obj, size) < 0)
			print_err("Error munmap", "");
	}
	return ret;
}

int main(int argc, char **argv)
{
	if (argc == 2) {
		if (read_obj(argv[1])) {
			return -1;
		}
	}
	else if (argc == 1) {
		if (read_obj("a.out\0")) {
			return -1;
		}
	}
	else {
		printf("Error: Too many args...\n");
	}
	return 0;
}
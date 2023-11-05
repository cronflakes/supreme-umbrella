#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#include "include/findsymbol.h"

int main(int argc, char **argv) {
	int fd;
	void *addr; 
	char *strtbl;
	struct stat s;
	
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;

	fd = open(argv[1], O_RDWR);
	if(fd == -1)
		return fd;

	if(stat(argv[1], &s) != 0) 
		return -1;

	addr = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(!addr) 
		return -1;
	
	//cast everything
	ehdr = (Elf64_Ehdr *)addr;
	phdr = (Elf64_Phdr *)(addr + ehdr->e_phoff);
	shdr = (Elf64_Shdr *)(addr + ehdr->e_shoff);

	//grab section string table
	strtbl = addr + (shdr[ehdr->e_shstrndx].sh_offset);

	//section headers
	for(int i = 0; i < ehdr->e_shnum; i++) {
		int index;
		if(shdr[i].sh_type == SHT_RELA) {
			//get relocation section
			if(strcmp(".rela.text", &strtbl[shdr[i].sh_name]) == 0) {
				printf("Name: %s\n", &strtbl[shdr[i].sh_name]);
				printf("Type: %d\n", shdr[i].sh_type);
				//get relocation entry based on symbol table offset
				index = get_symbol_index("__sys_accept4_file", addr, shdr, ehdr->e_shnum);
			}
		}	
		
	}

	close(fd);

	return 0;
}
	

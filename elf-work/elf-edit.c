#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#include "include/removerela.h"
#include "include/getsymstrtbl.h"

int main(int argc, char **argv) 
{
	int fd;
	long *offset;
	void *addr; 
	char *secstrtbl, *symstrtbl = NULL;
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

	//verify elf is relocatable
	if(ehdr->e_type != ET_REL) {
		printf("Input file must be relocatable\n");
		return -1;
	}
		
	//grab section and symbol string tables
	secstrtbl = addr + (shdr[ehdr->e_shstrndx].sh_offset);
	symstrtbl = get_symstrtbl(addr, shdr, secstrtbl, ehdr->e_shnum);

	//removals
	remove_symbol(addr, shdr, symstrtbl, ehdr->e_shnum, "__fdget", offset);
	remove_rela(addr, shdr, secstrtbl, ehdr->e_shnum, offset);

	close(fd);

	return 0;
}

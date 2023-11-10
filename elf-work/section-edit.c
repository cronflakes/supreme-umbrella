#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#include "include/findsymbol.h"
#include "include/removerela.h"
#include "include/getsymstrtbl.h"

#define DEBUG 1
#define WRITE 0

int main(int argc, char **argv) {
	int fd, relocs, marker, symbols = 0;
	void *addr; 
	char *sectstrtbl, *symstrtbl = NULL;
	struct stat s;
	
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;
	Elf64_Rela *rela;
	Elf64_Sym *sym;

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
		
	//grab section string table
	sectstrtbl = addr + (shdr[ehdr->e_shstrndx].sh_offset);

	//grab symbol string table
	symstrtbl = get_symstrtbl(addr, shdr, sectstrtbl, ehdr->e_shnum);

	//cycle through sections
	for(int i = 0; i < ehdr->e_shnum; i++) {
		//remove relocation entry
		if(shdr[i].sh_type == SHT_RELA) {
			if(strcmp(".rela.text", &sectstrtbl[shdr[i].sh_name]) == 0) {
#if DEBUG
				printf("Name: %s\n", &sectstrtbl[shdr[i].sh_name]);
				printf("Type: %d\n", shdr[i].sh_type);
#endif
				rela = (Elf64_Rela *)(addr + shdr[i].sh_offset);
				relocs = shdr[i].sh_size / sizeof(Elf64_Rela);
#if WRITE
				remove_rela(&shdr[i], rela, relocs, 0x2f);
#endif
			}
		}	



		//remove symbol entry
		if(shdr[i].sh_type == SHT_SYMTAB) {
			sym = (Elf64_Sym *)(addr + shdr[i].sh_offset);				
			symbols = shdr[i].sh_size / sizeof(Elf64_Sym);
			for(int j = 0; j < symbols; j++) {
				if(sym->st_info != STT_SECTION) {
					printf("Name: %s\n", &symstrtbl[sym->st_name]);
					printf("st_name: %d\n", sym->st_name);
					printf("st_info: %c\n", sym->st_info);
					printf("st_other: %c\n", sym->st_other);
					printf("st_shndx: %d\n", sym->st_shndx);
					printf("st_value: %llx\n", sym->st_value);
					printf("st_size: %d\n\n", sym->st_size);
				}
				sym++;
			}	
		}
	}

	close(fd);

	return 0;
}
	

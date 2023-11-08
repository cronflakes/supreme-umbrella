#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#include "include/findsymbol.h"

#define DEBUG 0

int main(int argc, char **argv) {
	int fd, relocs, marker = 0;
	void *addr; 
	char *strtbl;
	struct stat s;
	
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;
	Elf64_Rela *rela, *iter;

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

		if(shdr[i].sh_type == SHT_RELA) {
			if(strcmp(".rela.text", &strtbl[shdr[i].sh_name]) == 0) {
				#if DEBUG
				printf("Name: %s\n", &strtbl[shdr[i].sh_name]);
				printf("Type: %d\n", shdr[i].sh_type);
				#endif

				rela = (Elf64_Rela *)(addr + shdr[i].sh_offset);
				relocs = shdr[i].sh_size / sizeof(Elf64_Rela);
				iter = rela;
				for(int j = 0; j < relocs; j++) {
					#if DEBUG
					printf("r_offset: %016llx\n", iter->r_offset);
					printf("r_info: %016llx\n", iter->r_info);				
					printf("r_addend: %d\n", iter->r_addend);				
					#endif

					//check symbol table offset to catch match
					if((iter->r_info >> 32) == 0x2f) {
						memset(iter, 0, sizeof(Elf64_Rela));
						marker = j;			

						while(marker < relocs) {
							rela[marker] = rela[marker + 1];	
							marker++;
						}
												
						relocs--;
						shdr[i].sh_size = sizeof(Elf64_Rela) * relocs;
					}

					iter++;
				}
			}
		}	
		
	}

	close(fd);

	return 0;
}
	

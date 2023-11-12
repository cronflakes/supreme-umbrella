#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "include/removerela.h"
#include "include/getsymstrtbl.h"

//globals
void *addr; 
long symindex;
Elf64_Ehdr *ehdr;
Elf64_Shdr *shdr;
char *secstrtbl = NULL, *symstrtbl = NULL;

int main(int argc, char **argv) 
{
	int fd, opt;
	char *file = NULL, *symbol = NULL;
	struct stat s;

	struct option long_opts[] = { 
		{ "input", 1, NULL, 'i'},
		{ "remove-symbol", 1, NULL, 's'},
		{ "help", 0, NULL, 'h'}
	};

	while((opt = getopt_long(argc, argv, "hi:s:", long_opts, NULL)) != -1) {
		switch(opt) {
			case 'i':
				file = strdup(optarg);
				break;
			case 's': 
				symbol = strdup(optarg);
				break;
			case 'h':
				printf("Usage: %s --input <ELF relocatable> [--remove-symbol <symbol>]\n", argv[0]);
				break;
			default:
		}
	}

	fd = open(file, O_RDWR);
	if(fd == -1)
		return fd;
	if(stat(file, &s) != 0) 
		return -1;

	addr = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(!addr) 
		return -1;
	
	//cast everything
	ehdr = (Elf64_Ehdr *)addr;
	shdr = (Elf64_Shdr *)(addr + ehdr->e_shoff);
		
	//grab section and symbol string tables
	secstrtbl = addr + (shdr[ehdr->e_shstrndx].sh_offset);
	symstrtbl = get_symstrtbl(addr, shdr, secstrtbl, ehdr->e_shnum);

	//removals
	if(symbol != NULL) {
		remove_symbol(ehdr->e_shnum, symbol);
		remove_rela(ehdr->e_shnum, ".rela.text");
	}

	close(fd);

	return 0;
}

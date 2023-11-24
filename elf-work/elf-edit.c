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

void *addr; 
long symindex;
Elf64_Ehdr *ehdr;
Elf64_Shdr *shdr;
char *secstrtbl = NULL, *symstrtbl = NULL;

int main(int argc, char **argv) 
{
	int fd, opt;
	char *file = NULL, *symbol = NULL, *section = NULL;
	struct stat s;

	struct option long_opts[] = { 
		{ "input", 1, NULL, 'i'},
		{ "remove-symbol", 1, NULL, 's'},
		{ "add-section", 1, NULL, 'a'},
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
			case 'a': 
				section = strdup(optarg);
				break;
			case 'h':
				printf("Usage: %s --input <ELF relocatable> [--remove-symbol <symbol>] \
				[--add-section <section>]\n", argv[0]);
				break;
			default:
		}
	}

	fd = open(file, O_RDWR);
	if(fd == -1)
		return fd;
	if(stat(file, &s) != 0) 
		return -1;

	if(section != NULL) {
		if(ftruncate(fd, (off_t)(s.st_size + 64 + strlen(section) + 1)) == -1)
			return -1;
		
		close(fd);
		fd = open(file, O_RDWR);
	}

	addr = mmap(NULL, (size_t)(s.st_size + sizeof(Elf64_Shdr)), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(!addr) 
		return -1;
	
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

	//additions
	if(section != NULL)
	add_section(ehdr->e_shnum, section);
	close(fd);

	return 0;
}

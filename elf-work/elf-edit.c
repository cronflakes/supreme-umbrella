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
int livepatch;
long symindex;
Elf64_Ehdr *ehdr;
Elf64_Shdr *shdr;
char *secstrtbl, *symstrtbl;

int main(int argc, char **argv) {
	int fd, opt;
	char *file = NULL, *symbol = NULL, *section = NULL;
	struct stat s;

	struct option long_opts[] = { 
		{ "input", 1, NULL, 'i'},
		{ "livepatch", 1, NULL, 'l'},
		{ "help", 0, NULL, 'h'}
	};

	while((opt = getopt_long(argc, argv, "hl:i:s:", long_opts, NULL)) != -1) {
		switch(opt) {
			case 'i':
				file = strdup(optarg);
				break;
			case 'l':
				livepatch = 1;
				symbol = strdup(optarg);
				break;
			case 'h':
				printf("Usage: %s --input <ELF relocatable> [--livepatch <symbol>]\n", argv[0]);
				break;
			default:
		}
	}

	fd = open(file, O_RDWR);
	if(fd == -1)
		return fd;

	if(stat(file, &s) != 0) 
		return -1;

	if(ftruncate(fd, (off_t)(s.st_size + 1024)) == -1)
		return -1;
	else		
		close(fd);

	fd = open(file, O_RDWR);

	addr = mmap(NULL, (size_t)(s.st_size + sizeof(Elf64_Shdr)), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(!addr) 
		return -1;
	
	ehdr = (Elf64_Ehdr *)addr;
	shdr = (Elf64_Shdr *)(addr + ehdr->e_shoff);
		
	//grab section and symbol string tables
	secstrtbl = addr + (shdr[ehdr->e_shstrndx].sh_offset);
	symstrtbl = get_symstrtbl(addr, shdr, secstrtbl, ehdr->e_shnum);

	//livepatch 
	if(livepatch == 1 && symbol != NULL) {
		remove_symbol(ehdr->e_shnum, symbol);
		remove_rela(ehdr->e_shnum, ".rela.text");
		add_section(ehdr->e_shnum, ".klp.sym.vmlinux.text");
		add_symbol(ehdr->e_shnum, symbol, s.st_size + 1024);
		//needs symbol table offset
		add_rela(ehdr->e_shnum, ".klp.sym.vmlinux.text");
	}


	close(fd);
	return 0;
}

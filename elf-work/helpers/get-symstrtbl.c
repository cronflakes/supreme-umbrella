#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern char *symstrtbl;

char *get_symstrtbl(void *addr, Elf64_Shdr *shdr, char *secstrtbl, short sections) 
{
	char *symstrtbl = NULL;
	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type == SHT_STRTAB) {
			if(strncmp(&secstrtbl[shdr[i].sh_name], ".strtab", 7) == 0) {
				symstrtbl = addr + shdr[i].sh_offset;
				return symstrtbl;
			}
		}
	}

	if(symstrtbl == NULL) {
		printf("Symbol string table not found\n");
		exit(-1);
	}
}


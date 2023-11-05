#include <elf.h>
#include "include/findsymbol.h"

int get_symbol_index(char *s, void *addr, Elf64_Shdr *shdr, int sections)
{	
	Elf64_Sym *symbol;

	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type == SHT_SYMTAB) {
			symbol = (Elf64_Symbol *)(addr + (shdr[i].sh_offset);
		}
	}
	
	
}
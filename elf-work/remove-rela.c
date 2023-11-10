#include <elf.h>
#include <string.h>

void remove_rela(void *addr, Elf64_Shdr *shdr, char *secstrtbl, int sections, long *offset)
{
	int marker, relocs = 0;
	Elf64_Rela *rela, *iter;
	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type == SHT_RELA) {
			if(strcmp(".rela.txt", &secstrtbl[shdr[i].sh_name]) == 0) {
				rela = (Elf64_Rela *)(addr + shdr[i].sh_offset);
				iter = rela;
				relocs = shdr[i].sh_size / sizeof(Elf64_Rela);	
				for(int j = 0; j < relocs; j++) {
					if((iter->r_info >> 32) == *offset) {
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
}

void remove_symbol(void *addr, Elf64_Shdr *shdr, char *symstrtbl, int sections, char *symbol, long *offset) 
{
	int symbols;
	Elf64_Sym *sym;
	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type = SHT_SYMTAB) {
			sym = (Elf64_Sym *)(addr + shdr[i].sh_offset);
			symbols = shdr[i].sh_size / sizeof(Elf64_Sym);
			for(int j = 0; j < symbols; j++) {
				if(strncmp(symbol, &symstrtbl[sym->st_name], strlen(symbol)) == 0) {
					sym[j] = sym[j + 1];			
					*offset = j;
					symbols--;
					shdr[i].sh_size -= sizeof(Elf64_Sym);
				}

				sym++;
			}
		}
	}	
}

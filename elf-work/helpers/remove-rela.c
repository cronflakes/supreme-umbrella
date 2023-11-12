#include <elf.h>
#include <string.h>

extern void *addr;
extern long symindex;
extern Elf64_Ehdr *ehdr;
extern Elf64_Shdr *shdr;
extern char *secstrtbl;
extern char *symstrtbl;

void remove_rela(int sections, const char *section) 
{
	int marker, relocs = 0;
	Elf64_Rela *rela, *iter;
	for(int i = 0; i < sections; i++) {
		if((shdr[i].sh_type == SHT_RELA) && (strcmp(section, &secstrtbl[shdr[i].sh_name]) == 0)) {
			rela = (Elf64_Rela *)(addr + shdr[i].sh_offset);
			iter = rela;
			relocs = shdr[i].sh_size / sizeof(Elf64_Rela);	
			for(int j = 0; j < relocs; j++) {
				if((iter->r_info >> 32) == symindex) {
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

void remove_symbol(int sections, char *symbol) 
{
	int symbols, symindex;
	Elf64_Sym *sym;
	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type = SHT_SYMTAB) {
			sym = (Elf64_Sym *)(addr + shdr[i].sh_offset);
			symbols = shdr[i].sh_size / sizeof(Elf64_Sym);
			for(int j = 0; j < symbols; j++) {
				if(strncmp(symbol, &symstrtbl[sym->st_name], strlen(symbol)) == 0) {
					sym[j] = sym[j + 1];			
					symindex = j;
					symbols--;
					shdr[i].sh_size -= sizeof(Elf64_Sym);
				}

				sym++;
			}
		}
	}	
}

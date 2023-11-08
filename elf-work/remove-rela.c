#include <elf.h>

void remove_rela(Elf64_Shdr *section, Elf64_Rela *rela, int relocs, long offset)
{
	int marker = 0;
	Elf64_Rela *iter = rela;
	
	for(int j = 0; j < relocs; j++) {
		if((iter->r_info >> 32) == offset) {
			marker = j;
			while(marker < relocs) {
				rela[marker] = rela[marker + 1];
				marker++;
			}
		
			relocs--;
			section->sh_size = sizeof(Elf64_Rela) * relocs;
		}

		iter++;
	}
}
			
	

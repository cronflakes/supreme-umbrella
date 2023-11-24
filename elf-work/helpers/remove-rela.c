#include <elf.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

extern void *addr;
extern long symindex;
extern Elf64_Ehdr *ehdr;
extern Elf64_Shdr *shdr;
extern char *secstrtbl;
extern char *symstrtbl;

Elf64_Shdr *get_section(char *section) 
{
	for(int i = 0; i < ehdr->e_shnum; i++) {
		if(strncmp(&secstrtbl[shdr[i].sh_name], section, strlen(section)) == 0) {
			return (Elf64_Shdr *)(&shdr[i]);
		}
	}
}

void remove_rela(short sections, const char *section) 
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

void add_section(short sections, char *section) 
{
	int len = strlen(section);
	unsigned long long added_addr = 0;
	char *newentry = (char *)(secstrtbl + shdr[ehdr->e_shstrndx].sh_size);
	Elf64_Shdr *last = &shdr[sections - 1];
	Elf64_Shdr *rela = get_section(".rela.text");

	added_addr = last->sh_offset + last->sh_size;
	while((added_addr % 8) != 0) 
		added_addr++;

	last++;
	last->sh_name = shdr[ehdr->e_shstrndx].sh_size;
	last->sh_offset = added_addr;
	last->sh_type = SHT_RELA;
	last->sh_size = ehdr->e_shentsize;
	last->sh_addr = 0;
	last->sh_addralign = 8;
	last->sh_flags = SHF_ALLOC | SHF_OS_NONCONFORMING; 

	//copy defaults from .rela.text section
	if(rela != NULL) {
		last->sh_link = rela->sh_link;
		last->sh_info = rela->sh_info;
		last->sh_entsize = rela->sh_entsize;
	}

	Elf64_Shdr *ptr = (Elf64_Shdr *)malloc((ehdr->e_shnum * ehdr->e_shentsize) + ehdr->e_shentsize);
	memcpy(ptr, shdr, ehdr->e_shnum * ehdr->e_shentsize + ehdr->e_shentsize);
	strcpy(newentry, section);

	memcpy(addr + (ehdr->e_shoff + len + 1), ptr, (ehdr->e_shnum * ehdr->e_shentsize) + ehdr->e_shentsize);
	shdr = addr + (ehdr->e_shoff + len + 1);
	shdr[ehdr->e_shstrndx].sh_size += len + 1;
	ehdr->e_shnum = ++sections;
	ehdr->e_shoff += (len + 1);
	free(ptr);
}

void remove_symbol(short sections, char *symbol) 
{
	int symbols;
	Elf64_Sym *sym;
	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type == SHT_SYMTAB) {
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

void edit_symbol(short sections, char *symbol, int entry) 
{
	int symbols;
	char klp_symbol[256] = ".klp.sym.vmlinux.";
	Elf64_Sym *sym;
	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type == SHT_SYMTAB) {
			sym = (Elf64_Sym *)(addr + shdr[i].sh_offset);
			symbols = shdr[i].sh_size / sizeof(Elf64_Sym);
			for(int j = 0; j < symbols; j++) {
				if(strncmp(symbol, &symstrtbl[sym->st_name], strlen(symbol)) == 0) {
					switch(entry) {
						//st_name
						case 0:
							break;
						//st_info
						case 1:
							break;
						//st_other
						case 2:
							break;
						//st_shndx
						case 3:
							strcat(klp_symbol, symbol);
							strncpy(&symstrtbl[sym->st_name], klp_symbol, strlen(klp_symbol));
							sym->st_shndx = 0xff20;

							break;
						//st_value
						case 4:
							break;
						//st_size
						case 5:
							break;
						default:
							return;
					}
				}

				sym++;
			}
		}
	}	
}

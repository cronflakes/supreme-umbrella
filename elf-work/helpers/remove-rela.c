#include <elf.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

extern void *addr;
extern int livepatch;
extern long symindex;
extern Elf64_Ehdr *ehdr;
extern Elf64_Shdr *shdr;
extern char *secstrtbl;
extern char *symstrtbl;

Elf64_Sym livepatch_symbol;
Elf64_Rela livepatch_rela;

Elf64_Shdr *get_section(char *section) {
	for(int i = 0; i < ehdr->e_shnum; i++) {
		if(strncmp(&secstrtbl[shdr[i].sh_name], section, strlen(section)) == 0) {
			return (Elf64_Shdr *)(&shdr[i]);
		}
	}
}

void add_rela(short sections, const char *section) {
	Elf64_Rela *new;
	for(int i = 0; i < sections; i++) {
		if(strcmp(section, &secstrtbl[shdr[i].sh_name]) == 0) {
			new = (Elf64_Rela *)(addr + shdr[i].sh_offset);
			new->r_offset = livepatch_rela.r_offset;
			new->r_info = livepatch_rela.r_info;
			new->r_addend = livepatch_rela.r_addend;	
		}
	}
}

void remove_rela(short sections, const char *section) {
	int marker, relocs = 0;
	Elf64_Rela *rela, *iter;
	for(int i = 0; i < sections; i++) {
		if((shdr[i].sh_type == SHT_RELA) && (strcmp(section, &secstrtbl[shdr[i].sh_name]) == 0)) {
			rela = (Elf64_Rela *)(addr + shdr[i].sh_offset);
			iter = rela;
			relocs = shdr[i].sh_size / sizeof(Elf64_Rela);	
			for(int j = 0; j < relocs; j++) {
				if((iter->r_info >> 32) == symindex) {
					if(livepatch)
						livepatch_rela = *iter;

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

void add_section(short sections, char *section) {
	int length = strlen(section) + 1;
	unsigned long long added_addr = 0;
	char *newentry = (char *)(secstrtbl + shdr[ehdr->e_shstrndx].sh_size);
	Elf64_Shdr *last = &shdr[sections - 1];
	Elf64_Shdr *rela = get_section(".rela.text");
	Elf64_Shdr *tmp = (Elf64_Shdr *)malloc((ehdr->e_shnum * ehdr->e_shentsize) + ehdr->e_shentsize);

	added_addr = last->sh_offset + last->sh_size;
	while((added_addr % 8) != 0) 
		added_addr++;

	last++;
	last->sh_name = shdr[ehdr->e_shstrndx].sh_size;
	last->sh_offset = added_addr + 16;
	last->sh_type = SHT_RELA;
	last->sh_size = sizeof(Elf64_Rela);
	last->sh_addr = 0;
	last->sh_addralign = 8;
	last->sh_flags = SHF_ALLOC | SHF_OS_NONCONFORMING; 

	//copy defaults from .rela.text section
	if(rela != NULL) {
		last->sh_link = rela->sh_link;
		last->sh_info = rela->sh_info;
		last->sh_entsize = rela->sh_entsize;
	}
	
	//avoid section string table spilling into section headers
	memcpy(tmp, shdr, ehdr->e_shnum * ehdr->e_shentsize + ehdr->e_shentsize);
	strcpy(newentry, section);
	memcpy(addr + (ehdr->e_shoff + length + sizeof(Elf64_Rela)), tmp, (ehdr->e_shnum * ehdr->e_shentsize) + ehdr->e_shentsize);
	shdr = addr + (ehdr->e_shoff + sizeof(Elf64_Rela) + length);
	shdr[ehdr->e_shstrndx].sh_size += length;
	ehdr->e_shnum = ++sections;
	ehdr->e_shoff += (length + sizeof(Elf64_Rela));
	free(tmp);
}

void add_symbol(short sections, char *symbol, unsigned int filesize) {
	int symbols;
	Elf64_Sym *new;
	void *backup;
	char *ptr2;

	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type == SHT_SYMTAB) {
			symbols = shdr[i].sh_size / sizeof(Elf64_Sym);
			shdr[i].sh_size += sizeof(Elf64_Sym);
			new = (Elf64_Sym *)(addr + (shdr[i].sh_offset + shdr[i].sh_size));
			new = (Elf64_Sym *)&livepatch_symbol;
		}
	}

	for(int i = 0; i < sections; i++) {
		if((shdr[i].sh_type == SHT_STRTAB) && (strcmp(&secstrtbl[shdr[i].sh_name], ".strtab") == 0)) {
			backup = malloc(filesize);
			memcpy(backup, addr + shdr[i + 1].sh_offset, filesize);
			strncpy((char *)(symstrtbl + shdr[i].sh_size), symbol, strlen(symbol));
			shdr[i].sh_size += strlen(symbol);
			ptr2 = (char *)(addr + shdr[i].sh_size);
			while((*ptr2 % 8) != 0)
				ptr2++;

			memcpy(ptr2, backup, filesize);
			free(backup);
			break;
		}
	}
}

void remove_symbol(short sections, char *symbol) {
	int marker, symbols;
	Elf64_Sym *sym, *iter;
	for(int i = 0; i < sections; i++) {
		if(shdr[i].sh_type == SHT_SYMTAB) {
			sym = (Elf64_Sym *)(addr + shdr[i].sh_offset);
			symbols = shdr[i].sh_size / sizeof(Elf64_Sym);
			for(int j = 0; j < symbols; j++) {
				if(strcmp(symbol, &symstrtbl[sym->st_name]) == 0) {
					marker = j;
					iter = sym + 1;
					while(marker < symbols)	{
						*sym = *iter;
						sym++;
						iter++;
						marker++;
					}

					shdr[i].sh_size -= sizeof(Elf64_Sym);
					break;
				}

				sym++;
			}
		}
	}	
}


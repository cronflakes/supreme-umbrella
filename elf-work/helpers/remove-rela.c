#include <elf.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

extern void *addr;
extern int livepatch;
extern long symindex;
extern Elf64_Ehdr *ehdr;
extern Elf64_Shdr *shdr;
extern char *secstrtbl;
extern char *symstrtbl;

Elf64_Sym *livepatch_symbol = NULL;
Elf64_Rela *livepatch_rela = NULL;

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
					if(livepatch)
						livepatch_rela = iter;

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
	last->sh_offset = added_addr;
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
					symindex = j;
					if(livepatch)
						livepatch_symbol = sym;
						return;

					sym[j] = sym[j + 1];			
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
							strncpy(&symstrtbl[sym->st_name], symbol, strlen(symbol));

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

void livepatch_handler(char *livepatch_symbol) 
{
	//1. add new rela section
	add_section(ehdr->e_shnum, ".klp.sym.vmlinux.text");

	//2. remove rela entry from .rela.text
	remove_symbol(ehdr->e_shnum, livepatch_symbol);
	remove_rela(ehdr->e_shnum, livepatch_symbol);

	//3. add rela to new section
	Elf64_Rela *rela;
	for(int i = 0; i < ehdr->e_shnum; i++) {
		if((shdr[i].sh_type == SHT_RELA) && (strcmp(".klp.sym.vmlinux.text", &secstrtbl[shdr[i].sh_name]) == 0)) {
			rela = addr + shdr[i].sh_offset;
			if(livepatch_rela != NULL) {
				rela->r_offset = livepatch_rela->r_offset;
				rela->r_info = livepatch_rela->r_info;
				rela->r_addend = livepatch_rela->r_addend;
			}
			break;
		}
	}

	//4. modify symbol
	Elf64_Sym *sym;
	for(int i = 0; i < ehdr->e_shnum; i++) {
		if((shdr[i].sh_type == SHT_SYMTAB)) {
			sym = (Elf64_Sym *)(addr + shdr[i].sh_offset);
			for(int j = 0; j < (shdr[i].sh_size / sizeof(Elf64_Sym)); j++) {
				if(sym[j].st_name == symindex) {
					//put new string table index here
					sym[j].st_shndx = 0xff20;
					break;
				}
			}
		}
	}

	//symbol table

	char new_symbol[256];
	snprintf(new_symbol, 20 + strlen(livepatch_symbol), "%s%s%s", ".klp.sym.vmlinux.", livepatch_symbol, ",0");

	int remaining_sections = 0;
	int size_needed = 0;
	void *ptr = NULL;
	char *new_entry = NULL;

	for(int i = 0; i < ehdr->e_shnum; i++) {
		if(shdr[i].sh_type == SHT_STRTAB && (strcmp(".strtab", &secstrtbl[shdr[i].sh_name]))) {
			//modify string table
			new_entry = (char *)symstrtbl + shdr[i].sh_size;
			remaining_sections = ehdr->e_shnum - i;	
			for(int j = 0; j < remaining_sections; j++) {
				size_needed += shdr[i + j].sh_size;

			}

			ptr = malloc(size_needed + strlen(new_symbol));
			memcpy(ptr, addr + shdr[i + 1].sh_offset, size_needed + (ehdr->e_shnum * ehdr->e_shentsize));
			strcpy(new_entry, livepatch_symbol);
			memcpy(addr + (shdr[i + 1].sh_offset + 24), ptr, size_needed + (ehdr->e_shnum * ehdr->e_ehsize));
			shdr[i].sh_size += 16;		

		}
	}






	printf("Relocatable file adjusted for livepatch insertion\n");
}
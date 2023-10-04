#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int fd;
	Elf64_Addr orig_entry;
	void *addr = NULL;
	struct stat s;
	
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;

	fd = open(argv[1], O_RDWR);
	if(fd == -1)
		return fd;

	if(stat(argv[1], &s) != 0) 
		return -1;

	addr = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(!addr) 
		return -1;
	
	//cast everything
	ehdr = (Elf64_Ehdr *)addr;
	phdr = (Elf64_Phdr *)(addr + ehdr->e_phoff);
	shdr = (Elf64_Shdr *)(addr + ehdr->e_shoff);
	orig_entry = ehdr->e_entry;

	//program headers	
	for(int i = 0; i < ehdr->e_phnum; i++) {
		if(phdr[i].p_type == PT_NOTE) {
			phdr[i].p_type = PT_LOAD;	
			phdr[i].p_paddr = (Elf64_Addr)0x31540;
			phdr[i].p_vaddr = (Elf64_Addr)0x31540;
			//TODO - add offset, alignment, exec flags
		}
	}	

	//section headers
	for(int i = 0; i < ehdr->e_shnum; i++) {
		if(shdr[i].sh_type == SHT_NOTE) {
			shdr[i].sh_type = SHT_PROGBITS;
			shdr[i].sh_addr = (Elf64_Addr)0x31540;
			shdr[i].sh_size = 0xabc; 
			shdr[i].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
			shdr[i].sh_addralign = 0;
			//TODO - add offset
			break;
		}	
	}

	return 0;
}
	

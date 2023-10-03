#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int fd;
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

	ehdr = (Elf64_Ehdr *)addr;
	phdr = (Elf64_Phdr *)(addr + ehdr->e_phoff);
	shdr = (Elf64_Shdr *)(addr + ehdr->e_shoff);
	
	printf("Entry is at 0x%lx\n", ehdr->e_entry);
	printf("ehdr is %p\n", ehdr);
	printf("phdr is %p\n", phdr);
	printf("shdr is %p\n", shdr);
	
	for(int i = 0; i < ehdr->e_phnum; i++) {
		if(phdr[i].p_type == 4) {
			phdr[i].p_type = 1;	
		}
	}	

	return 0;
}
	


	
	

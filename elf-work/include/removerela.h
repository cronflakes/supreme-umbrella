#ifndef REMOVERELA_H_
#define REMOVERELA_H_

void remove_rela(void *, Elf64_Shdr *, char *, int, long *);
void remove_symbol(void *, Elf64_Shdr *, char *, int, char *, long *);

#endif

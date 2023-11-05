#ifndef FINDSYMBOL_H_
#define FINDSYMBOL_H_

int get_symbol_index(char *s, void *addr, Elf64_Shdr *shdr, int sections);

#endif

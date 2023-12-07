#ifndef REMOVERELA_H_
#define REMOVERELA_H_

void remove_rela(short, char *);
void remove_symbol(short, char *);
void edit_symbol(short, char *, int);
void add_section(short, char *);
void add_rela(short, char *);
void add_symbol(short, char *, unsigned int);

#endif

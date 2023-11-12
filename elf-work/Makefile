CC = gcc
OBJ = elf-edit.o helpers/remove-rela.o helpers/get-symstrtbl.o
CFLAGS =-g


elf-edit: $(OBJ)
	$(CC) -o $@ $^ 

clean:
	rm -rvf elf-edit $(OBJ)

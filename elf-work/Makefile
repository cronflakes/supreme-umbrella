CC = gcc
OBJ = section-edit.o remove-rela.o get-symstrtbl.o

section-edit: $(OBJ)
	$(CC) -o $@ $^

clean:
	rm -rvf section-edit $(OBJ)

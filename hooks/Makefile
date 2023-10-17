obj-m += hooks.o

hooks-y := accept/ice-cream.o
hooks-y += fakekallsyms.o

ccflags-y := -I ../include

all: 
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules

clean:
	make -C /lib/modules/`uname -r`/build M=$(PWD) clean

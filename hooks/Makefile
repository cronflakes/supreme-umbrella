obj-m += hooks.o

hooks-y := __sys_accept4.o fakekallsyms.o

ccflags-y := -Iinclude

all: 
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules

clean:
	make -C /lib/modules/`uname -r`/build M=$(PWD) clean

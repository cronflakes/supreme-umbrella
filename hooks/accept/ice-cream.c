#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/file.h>
#include <linux/livepatch.h>

#include "../include/fakekallsyms.h"

int (*__sys_accept4_file_fp)(struct file *, unsigned, struct sockaddr __user *, int __user *, int, unsigned long);

int __sys_accept4_2(int fd, struct sockaddr __user *upeer_sockaddr, int __user *upeer_addrlen, int flags)
{
	int ret = -EBADF;
	struct fd f;
	
	f = fdget(fd);
	if(f.file) {
		ret = __sys_accept4_file_fp(f.file, 0, upeer_sockaddr, upeer_addrlen, flags, rlimit(RLIMIT_NOFILE));
		fdput(f);	
	}

	pr_info("__sysaccept4 hooked - here is your file descriptor -> %d\n", ret);
	return ret;	
}

<<<<<<< HEAD
=======
unsigned long lookup_addr(const char *symbol)
{
	int ret;
	unsigned long addr;
	struct kprobe kp = { .symbol_name = symbol };

	ret = register_kprobe(&kp);
	if(ret == 0) {
		addr = (unsigned long)kp.addr;
		unregister_kprobe(&kp);
	} else {
		pr_info("REGISTER_KPROBE: %d\n", ret);
		return 0;
	}
	
	return addr;
}

>>>>>>> d8359c02ce4ce7cfce6cb89151a4645fddbd1c14
struct klp_func func = {
	.old_name = "__sys_accept4",
	.new_func = __sys_accept4_2
};

struct klp_object obj = {	
	.funcs = &func
};

struct klp_patch patch = {
	.mod = THIS_MODULE,
	.objs = &obj
};

int init_module(void)
{
	const char *symbol = "__sys_accept4_file";
	__sys_accept4_file_fp = (void *)lookup_addr(symbol);
	if(__sys_accept4_file_fp) {
		klp_enable_patch(&patch);	
	} 

	return 0;
}

void exit_module(void) {}

MODULE_LICENSE("GPL");
MODULE_INFO(livepatch, "Y");

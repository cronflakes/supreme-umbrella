#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/file.h>
#include <linux/livepatch.h>

#include "include/fakekallsyms.h"

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
	__sys_accept4_file_fp = kallsyms_lookup_name(symbol);
	if(__sys_accept4_file_fp) {
		klp_enable_patch(&patch);	
	} 

	return 0;
}

void exit_module(void) {}

MODULE_LICENSE("GPL");
MODULE_INFO(livepatch, "Y");
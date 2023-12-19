#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/file.h>
#include <linux/livepatch.h>
#include <uapi/linux/in.h>

#include "include/fakekallsyms.h"

int (*__sys_accept4_file_fp)(struct file *, unsigned, struct sockaddr __user *, int __user *, int, unsigned long);

int __sys_accept4_2(int fd, struct sockaddr __user *upeer_sockaddr, int __user *upeer_addrlen, int flags)
{
	int ret = -EBADF;
	pid_t pid = current->pid;
	struct fd f;
	struct sockaddr_in *addr_in = (struct sockaddr_in *)upeer_sockaddr;
	
	f = fdget(fd);
	if(f.file) {
		ret = __sys_accept4_file_fp(f.file, 0, upeer_sockaddr, upeer_addrlen, flags, rlimit(RLIMIT_NOFILE));
		fdput(f);	
	}

	if(addr_in != NULL)
		printk(KERN_ERR "hooked accept from %pI4 on port %d, here is your fd --> %d for pid --> %d", &(addr_in->sin_addr.s_addr), ntohs(addr_in->sin_port), ret, pid);

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

	__sys_accept4_file_fp = (void *)kallsyms_lookup_name(symbol);

	if(__sys_accept4_file_fp) {
		klp_enable_patch(&patch);	
	} 

	return 0;
}

void exit_module(void) {}

MODULE_LICENSE("GPL");
MODULE_INFO(livepatch, "Y");

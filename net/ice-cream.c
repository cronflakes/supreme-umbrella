#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/ftrace.h>
#include <linux/string.h>

void callback_func(unsigned long, unsigned long, struct ftrace_ops *, struct ftrace_regs *);

struct ftrace_ops ops = {
	.func = callback_func,
	.flags = FTRACE_OPS_FL_SAVE_REGS_IF_SUPPORTED
};

void callback_func(unsigned long ip, unsigned long parent_ip, struct ftrace_ops *ops, struct ftrace_regs *fregs) {
	printk(KERN_INFO "Calling callback function!!!\n");
	return;
}

int init_module(void) {
	int ret = ftrace_set_filter(&ops, "accept", strlen("accept"), 0);
	if(ret == 0) {
		register_ftrace_function(&ops);
	} else {
		return ret;
	}

	return 0;	
}

void cleanup_module(void) {
	unregister_ftrace_function(&ops);
}

MODULE_LICENSE("GPL");

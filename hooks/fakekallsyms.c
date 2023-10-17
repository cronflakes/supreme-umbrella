#include <linux/kprobes.h>
#include "include/fakekallsyms.h"

unsigned long kallsyms_lookup_name(const char *name)
{
	int ret;
	unsigned long addr;
	struct kprobe kp = { .symbol_name = name };

	ret = register_kprobe(&kp);
	if(ret == 0) {
		addr = (unsigned long)kp.addr;
		unregister_kprobe(&kp);
		return addr;
	}

	return 0;
}

MODULE_LICENSE("GPL");

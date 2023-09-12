#include <linux/lsm_hooks.h>


int my_hook(int family, int type, int protocol, int kern)
{	
	printk(KERN_ALERT "socket created!!!");
	return 0;
}

struct security_hook_list hooks[] = {
	LSM_HOOK_INIT(socket_create, my_hook),
};

int sockwatch_init(void)
{	
	security_add_hooks(hooks, ARRAY_SIZE(hooks), "sockwatch");
	return 0;
}


DEFINE_LSM(socketwatch) = {
	.name = "sockwatch",
	.init = sockwatch_init,
};

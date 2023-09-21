#include <linux/kernel.h>
#include <linux/module.h>


int init_module(void)
{
	int err = 0;
	struct socket *s;

	err = sock_create(AF_INET, SOCK_STREAM, 0, s); 	
	if(err)
		return err;
		
	
	
	
	return 0;
}

MODULE_LICENSE("GPL");

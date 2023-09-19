#include <linux/kernel.h>
#include <linux/module.h>


int init_module(void)
{
	int err = 0;
	struct socket *s;
	
	sock_create_kern(, AF_INET, SOCK_STREAM, 0, s); 	



MODULE_LICENSE("GPL");

#include <linux/module.h>
#include <linux/net.h>
#include <linux/socket.h>

const enum uids { 
	user1 = 1000, 
	user2 = 1001, 
	user3 = 1002,
};

int init_module(void)
{
	int err = 0;
	struct socket *s;
	struct msghdr *msg;

	err = sock_create(PF_PACKET, SOCK_RAW, 0, &s); 	
	if(err)
		return err;

	for(;;)
	{
		sock_recvmsg(s, msg, 0);				
		printk(KERN_INFO "%p\n", msg->msg_name);
	}	
	
	return 0;
}

MODULE_LICENSE("GPL");

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/random.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/string.h>

ssize_t flip_coin(struct file *, char __user *, size_t, loff_t *);

int major_number;
struct cdev cdev;
struct file_operations fops = { .owner = THIS_MODULE, .read = flip_coin };

ssize_t flip_coin(struct file *f, char __user *user, size_t size, loff_t *loff)
{
	int err = 0;
	u32 random = get_random_u32() % 2;
	char *arr[2] = { "heads", "tails" };
	char msg[32];
	pid_t ppid = current->real_parent->pid;
	
	snprintf(msg, sizeof(msg), "PID: %d flipped %s!!!", ppid, arr[random]);
	printk(KERN_INFO "%s\n", msg);

	return err;
}

int init_module(void)
{
	int err;
	dev_t dev;

	err = alloc_chrdev_region(&dev, 0, 1, "coin-toss");
	if(err != 0)
	{
		printk(KERN_ALERT "ALLOC_CHRDEV_REGION: %d\n", err);
		return err;
	}
	
	major_number = MAJOR(dev);
	cdev_init(&cdev, &fops);
	
	err = cdev_add(&cdev, dev, 1);
	if(err < 0)
	{
		printk(KERN_ALERT "CDEV_ADD: %d\n", err);
		return err;
	}

	printk(KERN_INFO "COIN MAJOR NUMBER: %d\n", major_number);			

	return err;
}	

void cleanup_module(void)
{
	cdev_del(&cdev);
	unregister_chrdev_region(major_number, 0);
}
	
MODULE_LICENSE("GPL");

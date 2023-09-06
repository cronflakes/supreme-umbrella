#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/random.h>
#include <linux/uaccess.h>

ssize_t flip_coin(struct file *, char __user *, size_t, loff_t *);

int major_number;
struct cdev cdev;
struct file_operations fops = { .owner = THIS_MODULE, .read = flip_coin };

ssize_t flip_coin(struct file *f, char __user *user, size_t size, loff_t *loff)
{
	int err = 0;
	u32 random = get_random_u32() % 2;
	char *arr[2] = { "heads\n", "tails\n" };
	
	// simple_read_from_buffer() - copy data from the buffer to userspace
	// using dummy variables to clean up messages
	pr_info("Calling flip coin: %p\t%d", arr, random);
	if(err < 0)
	{
		printk(KERN_ALERT "SIMPLE_READ_FROM_BUFFER: %d\n", err);
		return err;
	
	}

	return err;
}


int init_module(void)
{
	int err;
	dev_t dev;

	// alloc_chrdev_region() - register a range of char device numbers
	err = alloc_chrdev_region(&dev, 0, 1, "coin-toss");
	if(err != 0)
	{
		printk(KERN_ALERT "ALLOC_CHRDEV_REGION: %d\n", err);
		return err;
	}
	
	// #define MAJOR(dev)	((dev)>>8)
	major_number = MAJOR(dev);

	// cdev_init() - initialize cdev structure
	cdev_init(&cdev, &fops);
	
	// cdev_add() - add a char device to the system
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
	// cdev_del() - remove a cdev from the system
	cdev_del(&cdev);

	// unregister_chrdev_region - ungregister a range of device numbers
	unregister_chrdev_region(major_number, 0);
}
	
MODULE_LICENSE("GPL");

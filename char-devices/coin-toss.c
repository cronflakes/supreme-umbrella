#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/random.h>
#include <linux/uaccess.h>

int major_number;
struct file_operations fops = { .read = flip_coin };
struct cdev cdev;

size_t flip_coin(struct file *f, char __user *user, size_t size, loff_t *loff)
{
	u32 random = get_random_u32() % 2;
	char *arr[2] = { "heads\n", "tails\n" };
	
	//write to userspace here
		
		
	
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

	printk(KERN_INFO "COIN MAJOR NUMBER: %Dd\n", major_number);			

	return err;
}	

void cleanup_module(void)
{
	// cdev_del() - remove a cdev from the system
	cdev_del(&cdev);

	// unregister_chrdev_region - ungregister a rance of device numbers
	unregister_chrdev_region(major_number, 0);
}
	

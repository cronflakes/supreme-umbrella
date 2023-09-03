#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

int major_number;
struct file_operations fops = { .read = flip_coin };
struct cdev cdev;

int init_module(void)
{
	int err;
	dev_t dev;

	// alloc_chrdev_region() - register a range of char device numbers
	err = alloc_chrdev_region(&dev, 0, 1, "coin");
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

	// find solution to get major number into userspace to make character file (currently is in major_number global)
	// alloc_chrdev_region dynamically chooses major number. 
	// __register_chrdev() may be what you are looking for to set static major number.
			
	return err;
}	
	

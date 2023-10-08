#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/vmalloc.h>

static int size = 256;
module_param(size, int, 0664);

void *kmalloc_req(unsigned long s) 
{
	return kmalloc(s, GFP_KERNEL);
}	

unsigned long page_req(void)
{
	return __get_free_page(GFP_KERNEL);
}

void *vmalloc_req(unsigned long s)
{
	return vmalloc(s);
}

void get_locations(unsigned long s)
{
	void *vaddr_req1, *vaddr_req2;
	unsigned long vaddr_req3;
	unsigned long long paddr_req1, paddr_req2, paddr_req3;

	vaddr_req1 = kmalloc_req(size);
	vaddr_req2 = vmalloc_req(size);
	vaddr_req3 = page_req();

	paddr_req1 = virt_to_phys(vaddr_req1);
	paddr_req2 = virt_to_phys(vaddr_req2);
	paddr_req3 = virt_to_phys((void *)vaddr_req3);
	
	pr_info("Allocated (%d bytes); kmalloc() virtual address: 0x%016llx and physical address: 0x%016llx\n", size, vaddr_req1, paddr_req1);
	pr_info("Allocated (%d bytes); vmalloc() virtual address: 0x%016llx and physical address: 0x%016llx\n", size, vaddr_req2, paddr_req2);
	pr_info("Allocated (%d bytes); __get_free_page() virtual address: 0x%016llx and physical address: 0x%016llx\n", size, vaddr_req3, paddr_req3);
	
	kfree(vaddr_req1);
	vfree(vaddr_req2);
	free_page(vaddr_req3);	
}

int init_module(void)
{
	get_locations(size);
	return 0;
}

void exit_module(void) { pr_info("Exiting\n"); }

MODULE_LICENSE("GPL");

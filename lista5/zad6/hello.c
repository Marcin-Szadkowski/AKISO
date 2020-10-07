#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init hello_start(void)
{
	printk(KERN_INFO "HELLO WORLD\n");
	return 0;
}
static void __exit hello_end(void)
{
	printk(KERN_INFO "BYE\n");
}
module_init(hello_start);
module_exit(hello_end);

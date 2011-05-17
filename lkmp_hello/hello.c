#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

static int version = 0;
module_param(version, int, 0644);

/* init function */
static int __init hello_init (void)
{
	printk (KERN_INFO "hello_init: %d\n", version);
	return 0;
}

/* exit function */
static void __exit hello_exit (void)
{
	printk (KERN_INFO "hello_exit\n");
}

module_init (hello_init);
module_exit (hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zaicheng Qi zaichengqi@gmail.com");
MODULE_DESCRIPTION("a simple linux kernel module test");

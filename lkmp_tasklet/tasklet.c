#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>

static char *my_string = "hello from tasklet!";

static void my_tasklet_func(unsigned long data)
{
	printk(KERN_INFO "%s\n", *(char **)data);

	return;
}

DECLARE_TASKLET(my_tasklet, my_tasklet_func,
		(unsigned long)&my_string);

static int __init my_tasklet_init (void)
{
	tasklet_schedule(&my_tasklet);

	printk(KERN_INFO "my_tasklet module inited! %s\n", my_string);

	return 0;
}

static void __exit my_tasklet_exit(void)
{
	tasklet_kill(&my_tasklet);

	printk(KERN_INFO "my_tasklet module exited!\n");

	return;
}

module_init (my_tasklet_init);
module_exit (my_tasklet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zaicheng Qi zaichengqi@gmail.com");
MODULE_DESCRIPTION("a simple linux tasklet test");

/* a simple test for procfs of linux kernel */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#define PROCFS_NAME "my_proc_hello"

static struct proc_dir_entry *my_proc_file;

static int my_proc_read(char *buf,
			char **bufp,
			off_t off,
			int len,
			int *eof,
			void *data)
{
	int ret = -1;
	printk(KERN_INFO "my_proc_read(/proc/%s) called\n", PROCFS_NAME);

	if(off > 0){
		ret = 0;
	}else{
		ret = sprintf(buf, "hello world\n");
	}
	
	return ret;
}

static int __init my_proc_init(void)
{
	my_proc_file = create_proc_entry(PROCFS_NAME, 0644, NULL);
	printk(KERN_INFO "Creating proc entry\n");

	if(!my_proc_file){
		remove_proc_entry(PROCFS_NAME, NULL);
		printk(KERN_INFO "Error: could not initialize /proc/%s\n",
		       PROCFS_NAME);
		return -ENOMEM;
	}

	my_proc_file->read_proc = my_proc_read;
	/* my_proc_file->owner = THIS_MODULE; */
	my_proc_file->mode = S_IFREG | S_IRUGO;
	my_proc_file->uid = 0;
	my_proc_file->gid = 0;
	my_proc_file->size = 37;

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	return 0;
}

static void __exit my_proc_exit(void)
{
	remove_proc_entry(PROCFS_NAME, NULL);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

module_init(my_proc_init);
module_exit(my_proc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Qi nick.qi@teleca.com");
MODULE_DESCRIPTION("a simple test for proc fs of linux");

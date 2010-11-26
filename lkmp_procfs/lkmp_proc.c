/* a simple test for procfs of linux kernel */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define PROCFS_NAME "my_proc_hello"
#define PROCFS_MAX_SIZE 1024

static struct proc_dir_entry *my_proc_file;

static char proc_buffer[PROCFS_MAX_SIZE];
static unsigned long proc_buffer_size = 0;

/* proc read function */
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
		memcpy(buf, proc_buffer, proc_buffer_size);
		ret = proc_buffer_size;
	}

	return ret;
}

/* proc write function */
static int my_proc_write(struct file *filp,
			 const char *buf,
			 unsigned long count,
			 void *data)
{
	proc_buffer_size = count;
	if(proc_buffer_size > PROCFS_MAX_SIZE){
		proc_buffer_size = PROCFS_MAX_SIZE;
	}

	printk(KERN_INFO "my_proc_write(/proc/%s) called\n", PROCFS_NAME);
	if(copy_from_user(proc_buffer, buf, proc_buffer_size)){
		return -EFAULT;
	}

	return proc_buffer_size;
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
	my_proc_file->write_proc = my_proc_write;
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

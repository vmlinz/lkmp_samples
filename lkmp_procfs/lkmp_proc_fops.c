#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

#include <asm/uaccess.h>
#include <asm/current.h>

#define PROC_ENTRY_NAME "my_proc_fops"
#define PROCFS_MAX_SIZE 2048

static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static struct proc_dir_entry *my_proc_entry;

static ssize_t my_proc_fops_read(struct file *filp,
				 char *buf,
				 size_t len,
				 loff_t *off)
{
	static int finished = 0;
	if(finished){
		printk(KERN_INFO "%s: file end\n", __func__);
		finished = 0;
		return 0;
	}

	finished = 1;

	if(copy_to_user(buf, procfs_buffer, procfs_buffer_size)){
		return -EFAULT;
	}

	printk(KERN_INFO "%s: read %lu bytes\n", __func__, procfs_buffer_size);

	return procfs_buffer_size;
}

static ssize_t my_proc_fops_write(struct file *filp,
				  const char *buf,
				  size_t len,
				  loff_t *off)
{
	if(len > PROCFS_MAX_SIZE){
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}else{
		procfs_buffer_size = len;
	}

	if(copy_from_user(procfs_buffer, buf, procfs_buffer_size)){
		return -EFAULT;
	}

	printk(KERN_INFO "%s: write %lu bytes\n",
	       __func__, procfs_buffer_size);
	return procfs_buffer_size;
}

static int my_proc_fops_open(struct inode *inode,
			     struct file *filp)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int my_proc_fops_release(struct inode *inode,
				struct file *filp)
{
	module_put(THIS_MODULE);
	return 0;
}

static int my_proc_inode_perm(struct inode *inode,
			      int op)
{
	printk(KERN_INFO "inode op mode = %x\n", op);
	if((op & 0x4 || op & 0x40 || op & 0x400)
	   || ((op & 0x2 || op & 0x20 || op & 0x200)
	       && current->cred->euid == 0))
		return 0;

	return -EACCES;
}

static struct file_operations my_proc_file_ops = {
	.read = my_proc_fops_read,
	.write = my_proc_fops_write,
	.open = my_proc_fops_open,
	.release = my_proc_fops_release
};

static struct inode_operations my_proc_indoe_ops = {
	.permission = my_proc_inode_perm
};

static int __init my_proc_init(void)
{
	my_proc_entry = create_proc_entry(PROC_ENTRY_NAME, 0644, NULL);

	if(!my_proc_entry){
		printk(KERN_INFO "Error: init proc file /proc/%s failed\n",
		       PROC_ENTRY_NAME);
		return -ENOMEM;
	}

	my_proc_entry->proc_iops = &my_proc_indoe_ops;
	my_proc_entry->proc_fops = &my_proc_file_ops;
	my_proc_entry->mode = S_IFREG | S_IRUGO | S_IWUSR;
	my_proc_entry->uid = 0;
	my_proc_entry->gid = 0;
	my_proc_entry->size = 80;

	printk(KERN_INFO "/proc/%s created\n", PROC_ENTRY_NAME);
	return 0;
}

static void __exit my_proc_exit(void)
{
	remove_proc_entry(PROC_ENTRY_NAME, NULL);
	printk(KERN_INFO "/proc/%s removed\n", PROC_ENTRY_NAME);
}

module_init(my_proc_init);
module_exit(my_proc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Qi nick.qi@teleca.com");
MODULE_DESCRIPTION("simple test for proc fs with inode operations");

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_ENTRY_NAME "my_seq"

static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long my_counter = 0;

	printk(KERN_INFO "%s: called\n", __func__);
	if(*pos == 0){
		return &my_counter;
	}else{
		*pos = 0;
		return NULL;
	}
}

static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	printk(KERN_INFO "%s: called\n", __func__);
	unsigned long *tmp_v = (unsigned long *)v;
	(*tmp_v)++;
	(*pos)++;

	return NULL;
}

static void my_seq_stop(struct seq_file *s, void *v)
{
	printk(KERN_INFO "%s: called\n", __func__);
}

static int my_seq_show(struct seq_file *s, void *v)
{
	loff_t *spos = (loff_t *)v;

	printk(KERN_INFO "%s: pos = %Ld\n", __func__, *spos);
	seq_printf(s, "%Ld\n", *spos);
	return 0;
}

static struct seq_operations my_seq_ops = {
	.start = my_seq_start,
	.next = my_seq_next,
	.stop = my_seq_stop,
	.show = my_seq_show
};

static int my_seq_fops_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
}

static struct file_operations my_file_ops = {
	.owner = THIS_MODULE,
	.open = my_seq_fops_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

static int __init my_seq_init(void)
{
	struct proc_dir_entry *entry;

	printk(KERN_INFO "%s: called\n", __func__);
	entry = create_proc_entry(PROC_ENTRY_NAME, 0, NULL);
	if(entry){
		entry->proc_fops = &my_file_ops;
	}

	return 0;
}

static void __exit my_seq_exit(void)
{
	printk(KERN_INFO "%s: called\n", __func__);
	remove_proc_entry(PROC_ENTRY_NAME, NULL);
}

module_init(my_seq_init);
module_exit(my_seq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Qi nick.qi@teleca.com");
MODULE_DESCRIPTION("simple test for proc system with seq file");

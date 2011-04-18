/* create a read-only char device for test */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
/* new api to create device node automaticly through udev  */
#include <linux/device.h>

/* function prototypes */
static int __init my_chardev_init(void);
static void __exit my_chardev_exit(void);
static int my_chardev_open(struct inode *, struct file *);
static int my_chardev_release(struct inode *, struct file *);
static ssize_t my_chardev_read(struct file *, char *, size_t, loff_t *);
static ssize_t my_chardev_write(struct file *, const char *, size_t, loff_t *);

/* power management functions */
static int my_class_suspend(struct device *dev, pm_message_t state);
static int my_class_resume(struct device *dev);

#define SUCCESS 0
#define BUF_LEN 80
#define DEVICE_NAME "my_chardev"

static int my_major = 550;
static int my_minor = 0;
static dev_t my_devno = 0;

static struct cdev my_cdev;

/* driver class */
static struct class *my_class;

static int s_device_opened = 0;

static char msg[BUF_LEN];
static char *msg_ptr;

static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = my_chardev_read,
	.write = my_chardev_write,
	.open = my_chardev_open,
	.release = my_chardev_release
};

/* the module init function */
static int __init my_chardev_init(void)
{
	int result;

	/* register char device number */
	my_devno = MKDEV(my_major, my_minor);
	result = register_chrdev_region(my_devno, 1, "my_dev");
	if (result < 0){
		printk(KERN_ALERT "failed to register char device : %d\n",
			my_major);
		return result;
	}

	/* init cdev struct and add cdev to driver model */
	cdev_init(&my_cdev, &my_fops);
	result = cdev_add(&my_cdev, my_devno, 1);
	if (result){
		printk(KERN_ALERT "failed to add cdev to system: %d",
		       result);
		return result;
	}

	/* create class under /sysfs */
	my_class = class_create(THIS_MODULE, "my_class");

	if(IS_ERR(my_class)){
		printk(KERN_ALERT "Err: failed to create class for device");
		return -1;
	}

	my_class->suspend = my_class_suspend;
	my_class->resume = my_class_resume;

	/* register device in sysfs, which will trigger udev to create
	 * device node */
	device_create(my_class, NULL, MKDEV(my_major, my_minor), NULL, "%s%d", DEVICE_NAME, 0);

	printk(KERN_INFO "%s: device registered in system", DEVICE_NAME);

	return SUCCESS;
}

/* cleanup function */
static void __exit my_chardev_exit(void)
{
	cdev_del(&my_cdev);
	device_destroy(my_class, my_devno);
	class_destroy(my_class);

	unregister_chrdev_region(my_devno, 1);

	printk(KERN_INFO "%s: %s to unregister driver", DEVICE_NAME, __func__);
}

/* file operations */

/* open */
static int my_chardev_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (s_device_opened)
		return -EBUSY;

	s_device_opened++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/* release */
static int my_chardev_release(struct inode *inode, struct file *file)
{
	s_device_opened--;
	module_put(THIS_MODULE);

	return SUCCESS;
}

/* read */
static ssize_t my_chardev_read(struct file *file, char *buf,
			       size_t len, loff_t *off)
{
	int bytes_read = 0;

	if(*msg_ptr == 0)
		return 0;

	while (len && *msg_ptr){
		put_user(*(msg_ptr++), buf++);
		len--;
		bytes_read++;
	}

	return bytes_read;
}

/* write */
static ssize_t my_chardev_write(struct file *file, const char *buf,
				size_t len, loff_t *off)
{
	printk(KERN_INFO "Sorry, this operation isn't supported\n");

	return -EINVAL;
}


static int my_class_suspend(struct device *dev, pm_message_t state)
{
	printk(KERN_INFO "%s, suspending device: %s, state: %d",
		dev->class->name, dev_name(dev), state.event);
	return 0;
}

static int my_class_resume(struct device *dev)
{
	printk(KERN_INFO "%s, resuming device: %s",
		dev->class->name, dev_name(dev));
	return 0;
}

/* register module init and exit functions */
module_init(my_chardev_init);
module_exit(my_chardev_exit);

/* module information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Qi nick.qi@teleca.com");
MODULE_DESCRIPTION("A test for character device driver");

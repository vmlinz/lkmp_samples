#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define MYDEVNAME "my_hello"
#define DEFAULT_MSG "Hello World\n"

static dev_t dev;
static struct class *hello_class;
static struct device *hello_device;
static struct cdev *hello_cdev;
static char hello_buf[80];

static ssize_t hello_show(struct device *dev,
			  struct device_attribute *attr,
			  char *buf)
{
	return snprintf(buf, sizeof(hello_buf), "%s", hello_buf);
}

static ssize_t hello_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	memset(hello_buf, 0, sizeof(hello_buf));
	return snprintf(hello_buf, sizeof(hello_buf), "%s", buf);
}

/* define dev_attr_hello */
static DEVICE_ATTR(hello, 0666, hello_show, hello_store);

static int __init hello_init(void)
{
	int error;

	error = alloc_chrdev_region(&dev, 0, 2, "my_hello");

	if(error){
		printk(KERN_INFO "my_hello: alloc_chrdev_region failed!\n");
		goto out;
	}

	hello_cdev = cdev_alloc();
	if(hello_cdev == NULL){
		printk(KERN_INFO "my_hello: cdev_alloc failed!\n");

		error = -ENOMEM;
		goto out_chrdev;
	}

	hello_cdev->ops = NULL;
	hello_cdev->owner = THIS_MODULE;
	error = cdev_add(hello_cdev, dev, 1);
	if(error){
		printk(KERN_INFO "my_hello: cdev_add failed!\n");
		goto out_cdev;
	}

	hello_class = class_create(THIS_MODULE, MYDEVNAME);
	if(IS_ERR(hello_class)){
		error = PTR_ERR(hello_class);
		goto out_chrdev;
	}

	hello_device = device_create(hello_class, NULL, dev, NULL, MYDEVNAME);
	error = device_create_file(hello_device, &dev_attr_hello);

	if(error){
		printk(KERN_INFO "my_hello: failed to create sysfs node!\n");
	}

	memset(hello_buf, 0, sizeof(hello_buf));
	memcpy(hello_buf, DEFAULT_MSG, strlen(DEFAULT_MSG));
	printk(KERN_INFO "my_hello: Hello World!\n");

	return 0;

out_cdev:
	cdev_del(hello_cdev);
out_chrdev:
	unregister_chrdev_region(hello_cdev->dev, 2);
out:
	return error;
}

static void __exit hello_exit(void)
{
	device_remove_file(hello_device, &dev_attr_hello);
	device_destroy(hello_class, dev);
	class_destroy(hello_class);

	unregister_chrdev_region(hello_cdev->dev, 2);
	cdev_del(hello_cdev);
	printk(KERN_INFO "my_hello: Goodbye World!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Nick Qi nick.qi@teleca.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple module test of creation of device nodes");

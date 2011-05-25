/*
 * simple block device driver test
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/blkdev.h>

#define SBD_BYTES (16 * 1024 * 1024)

static struct gendisk *sbd = NULL;
static struct request_queue * sbd_rq = NULL;
static int sbd_major = 0;
unsigned char sbd_data[SBD_BYTES];

static struct block_device_operations sbd_ops =
{
	.owner           = THIS_MODULE,
};

static void sbd_request_func(struct request_queue *q)
{
	struct request *req;

	while((req = blk_fetch_request(q)) != NULL)
	{
		if (req->cmd_type != REQ_TYPE_FS) {
			printk (KERN_NOTICE "Skip non-fs request\n");

			__blk_end_request_cur(req, -EIO);
			continue;
		}

		if(rq_data_dir(req) == WRITE)
			memcpy(sbd_data + (blk_rq_pos(req) << 9), req->buffer,
				blk_rq_bytes(req));
		else
			memcpy(req->buffer, sbd_data + (blk_rq_pos(req) << 9),
				blk_rq_bytes(req));

		__blk_end_request_cur(req, 0);
	}
}


static int __init sbd_init(void)
{
	int ret;

	printk(KERN_INFO "%s\n", __func__);

	sbd_major = register_blkdev(sbd_major, "sbd");
	if(sbd_major <= 0){
		printk(KERN_WARNING "sdb: unable to get major number\n");
		return -EBUSY;
	}

	sbd = alloc_disk(1);
	if(!sbd)
	{
		ret = -ENOMEM;
		goto err_alloc_disk;
	}

	sbd_rq = blk_init_queue(sbd_request_func, NULL);
	if(sbd_rq == NULL)
	{
		ret = -ENOMEM;
		goto err_alloc_queue;
	}

	sbd->major = sbd_major;
	sbd->first_minor = 0;
	sbd->fops = &sbd_ops;
	sbd->queue = sbd_rq;
	snprintf(sbd->disk_name, 32, "sbd");
	set_capacity(sbd, SBD_BYTES >> 9);

	add_disk(sbd);

	return 0;
err_alloc_queue:
	blk_cleanup_queue(sbd_rq);
err_alloc_disk:
	return ret;
}

static void __exit sbd_exit(void)
{
	printk(KERN_INFO "%s\n", __func__);

	if(sbd)
	{
		del_gendisk(sbd);
		put_disk(sbd);
	}

	if(sbd_rq)
		blk_cleanup_queue(sbd_rq);

	unregister_blkdev(sbd_major, "sbd");
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Zaicheng QI vmlinz@gmail.com");
MODULE_DESCRIPTION("simple block device driver for test");

module_init(sbd_init);
module_exit(sbd_exit);

/*
 * simple block device driver test
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/hdreg.h>

#include <linux/radix-tree.h>

#define SBD_MEM_RADIX
#define SBD_BYTES (16 * 1024 * 1024)

static struct gendisk *sbd = NULL;
static struct request_queue * sbd_rq = NULL;
/* static struct spinlock_t sbd_lock; */

static int sbd_major = 0;

#ifdef SBD_MEM_RADIX
static struct radix_tree_root sbd_data;
#else
unsigned char sbd_data[SBD_BYTES];
#endif

#ifdef SBD_MEM_RADIX
/* alloc and free memory for disk using radix tree and page allocator
 */

/** free memory allocated for disk
 * @param none
 * @ret none
 */
void sbd_free_diskmem(void)
{
	int i;
	void *p;

	for(i = 0; i < (SBD_BYTES + PAGE_SIZE - 1) >> PAGE_SHIFT; i++){
		p = radix_tree_lookup(&sbd_data, i);
		radix_tree_delete(&sbd_data, i);
		free_page((unsigned long)p);
	}
}

/** alloc memory for disk using page allocator
 * @param none
 * @ret int 0 for success, non-zero for failure
 */
int sbd_alloc_diskmem(void)
{
	int ret;
	int i;
	void *p;

	INIT_RADIX_TREE(&sbd_data, GFP_KERNEL);

	for(i = 0; i < (SBD_BYTES + PAGE_SIZE - 1) >> PAGE_SHIFT; i++){
		p = (void *)__get_free_page(GFP_KERNEL);
		if(!p){
			ret = -ENOMEM;
			goto err_alloc;
		}

		ret = radix_tree_insert(&sbd_data, i, p);
		if(IS_ERR_VALUE(ret)){
			goto err_radix_tree_insert;
		}
	}
	return 0;

err_radix_tree_insert:
	free_page((unsigned long)p);
err_alloc:
	sbd_free_diskmem();

	return ret;
}
#endif

static int sbd_getgeo(struct block_device *bd, struct hd_geometry *geo);

static struct block_device_operations sbd_ops =
{
	.owner           = THIS_MODULE,
	.getgeo          = sbd_getgeo,
};

static int sbd_getgeo(struct block_device *bd, struct hd_geometry *geo)
{
	if(SBD_BYTES < 16 * 1024 * 1024){
		geo->heads = 1;
		geo->sectors = 1;
	}else if(SBD_BYTES < 512 * 1024 * 1024){
		geo->heads = 1;
		geo->sectors = 32;
	}else if(SBD_BYTES < 16ULL * 1024 * 1024 * 1024){
		geo->heads = 32;
		geo->sectors = 32;
	}else{
		geo->heads = 255;
		geo->sectors = 63;
	}

	geo->cylinders = SBD_BYTES >> 9 / geo->heads / geo->sectors;

	return 0;
}

#ifndef SBD_MEM_RADIX
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

		if (((blk_rq_pos(req) << 9) + blk_rq_bytes(req)) > SBD_BYTES) {
			printk (KERN_INFO "out of disk boundary\n");

			__blk_end_request_cur(req, -EIO);
			break;
		}

		printk (KERN_INFO "%s, rq_pos << 9 = %lu, rq_bytes = %lu\n",
			(rq_data_dir(req) == WRITE) ? "WRITE" : "READ",
			(unsigned long)(blk_rq_pos(req) << 9),
			(unsigned long)blk_rq_bytes(req));

		if(rq_data_dir(req) == WRITE)
			memcpy(sbd_data + (blk_rq_pos(req) << 9), req->buffer,
				blk_rq_bytes(req));
		else
			memcpy(req->buffer, sbd_data + (blk_rq_pos(req) << 9),
				blk_rq_bytes(req));

		__blk_end_request_cur(req, 0);
	}
}
#endif

static int sbd_make_request(struct request_queue *q, struct bio *bio)
{
	struct bio_vec *bvec;
	int i;
	unsigned long long dsk_offset;

	dsk_offset = bio->bi_sector * 512;

	bio_for_each_segment(bvec, bio, i){
		unsigned int count_done, count_current;
		void *dsk_mem;
		void *iovec_mem;

		iovec_mem = kmap(bvec->bv_page) + bvec->bv_offset;

		count_done = 0;
		while(count_done < bvec->bv_len){
			count_current = min(bvec->bv_len - count_done,
					PAGE_SIZE -
					(dsk_offset + count_done) % PAGE_SIZE);
			dsk_mem = radix_tree_lookup(&sbd_data,
						(dsk_offset + count_done) / PAGE_SIZE);
			dsk_mem += (dsk_offset + count_done) % PAGE_SIZE;

			switch(bio_rw(bio)){
			case READ:
			case READA:
				memcpy(iovec_mem, dsk_mem, count_current);
				break;
			case WRITE:
				memcpy(dsk_mem, iovec_mem, count_current);
				break;
			}
			count_done += count_current;

		}
		kunmap(bvec->bv_page);
		dsk_offset += bvec->bv_len;
	}

	bio_endio(bio, 0);
	return 0;
}

static int __init sbd_init(void)
{
	int ret;
	/* struct elevator_queue *oe; */

	printk(KERN_INFO "%s\n", __func__);

	sbd_major = register_blkdev(sbd_major, "sbd");
	if (sbd_major <= 0) {
		printk(KERN_WARNING "sdb: unable to get major number\n");
		return -EBUSY;
	}

	sbd = alloc_disk(1);
	if (!sbd) {
		ret = -ENOMEM;
		goto err_alloc_disk;
	}

	/* sbd_rq = blk_init_queue(sbd_request_func, NULL); */
	sbd_rq = blk_alloc_queue(GFP_KERNEL);
	if (sbd_rq == NULL) {
		ret = -ENOMEM;
		goto err_alloc_queue;
	}

	blk_queue_make_request(sbd_rq, sbd_make_request);

	/* oe = sbd_rq->elevator;
	if (IS_ERR_VALUE(elevator_init(sbd_rq, "noop")))
		printk (KERN_INFO "failed to switch elevator\n");
	else
		elevator_exit (oe);
	 */

#ifdef SBD_MEM_RADIX
	ret = sbd_alloc_diskmem();
	if(IS_ERR_VALUE(ret)){
		goto err_alloc_diskmem;
	}
#endif

	sbd->major = sbd_major;
	sbd->first_minor = 0;
	sbd->fops = &sbd_ops;
	sbd->queue = sbd_rq;
	snprintf(sbd->disk_name, 32, "sbd");
	set_capacity(sbd, SBD_BYTES >> 9);

	add_disk(sbd);

	return 0;

#ifdef SBD_MEM_RADIX
err_alloc_diskmem:
	put_disk(sbd);
#endif
err_alloc_queue:
	blk_cleanup_queue(sbd_rq);
err_alloc_disk:
	return ret;
}

static void __exit sbd_exit(void)
{
	printk(KERN_INFO "%s\n", __func__);

	if (sbd) {
		del_gendisk(sbd);
		put_disk(sbd);
	}

#ifdef SBD_MEM_RADIX
		sbd_free_diskmem();
#endif

	if(sbd_rq)
		kobject_put (&sbd_rq->kobj);
		/* blk_cleanup_queue(sbd_rq); */

	unregister_blkdev(sbd_major, "sbd");
}

module_init(sbd_init);
module_exit(sbd_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Zaicheng QI vmlinz@gmail.com");
MODULE_DESCRIPTION("simple block device driver for test");

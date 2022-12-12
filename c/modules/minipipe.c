/*
 * minipipe.c -- module implementation
 *
 * Example module which creates a virtual device driver.
 * Circular buffer (kfifo) is used to store received data (with write) and
 * reply with them on read operation.
 *
 * Copyright (C) 2021 Leonardo Jelenkovic
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form.
 * No warranty is attached.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/kfifo.h>
#include <linux/log2.h>

#include "config.h"

/* Buffer size */
static int buffer_size = BUFFER_SIZE;
static int max_threads = MAX_THREADS;

/* Parameter buffer_size can be given at module load time */
module_param(buffer_size, int, S_IRUGO);
MODULE_PARM_DESC(buffer_size, "Buffer size in bytes, must be a power of 2");

MODULE_PARM_DESC(max_threads, "Number of maximum threads that can read or write from this pipe");

MODULE_AUTHOR(AUTHOR);
MODULE_LICENSE(LICENSE);

struct minipipe_dev *Minipipe = NULL;
struct buffer *Buffer = NULL;
static dev_t Dev_no = 0;

/* prototypes */
static struct buffer *buffer_create(size_t, int *);
static void buffer_delete(struct buffer *);
static struct minipipe_dev *minipipe_create(dev_t, struct file_operations *,
											struct buffer *, int *);
static void minipipe_delete(struct minipipe_dev *);
static void cleanup(void);
static void dump_buffer(struct buffer *);

static int minipipe_open(struct inode *, struct file *);
static int minipipe_release(struct inode *, struct file *);
static ssize_t minipipe_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t minipipe_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations minipipe_fops = {
	.owner = THIS_MODULE,
	.open = minipipe_open,
	.release = minipipe_release,
	.read = minipipe_read,
	.write = minipipe_write};

/* init module */
static int __init minipipe_module_init(void)
{
	int retval;
	struct buffer *buffer;
	struct minipipe_dev *minipipe;
	dev_t dev_no = 0;

	printk(KERN_NOTICE "Module 'minipipe' started initialization\n");

	/* get device number(s) */
	retval = alloc_chrdev_region(&dev_no, 0, 1, DRIVER_NAME);
	if (retval < 0)
	{
		printk(KERN_WARNING "%s: can't get major device number %d\n",
			   DRIVER_NAME, MAJOR(dev_no));
		return retval;
	}

	/* create a buffer */
	/* buffer size must be a power of 2 */
	if (!is_power_of_2(buffer_size))
	{
		buffer_size = roundup_pow_of_two(buffer_size);
	}
	buffer = buffer_create(buffer_size, &retval);
	if (!buffer)
	{
		goto no_driver;
	}
	Buffer = buffer;

	/* create a device */
	minipipe = minipipe_create(dev_no, &minipipe_fops, buffer, &retval);
	if (!minipipe)
	{
		goto no_driver;
	}

	printk(KERN_NOTICE "Module 'minipipe' initialized with major=%d, minor=%d\n",
		   MAJOR(dev_no), MINOR(dev_no));

	Minipipe = minipipe;
	Dev_no = dev_no;

	return 0;

no_driver:
	cleanup();

	return retval;
}

static void cleanup(void)
{
	if (minipipe)
	{
		minipipe_delete(minipipe);
	}
	if (Buffer)
	{
		buffer_delete(Buffer);
	}
	if (Dev_no)
	{
		unregister_chrdev_region(Dev_no, 1);
	}
}

/* called when module exit */
static void __exit minipipe_module_exit(void)
{
	printk(KERN_NOTICE "Module 'minipipe' started exit operation\n");
	cleanup();
	printk(KERN_NOTICE "Module 'minipipe' finished exit operation\n");
}

module_init(minipipe_module_init);
module_exit(minipipe_module_exit);

/* Create and initialize a single buffer */
static struct buffer *buffer_create(size_t size, int *retval)
{
	struct buffer *buffer = kmalloc(sizeof(struct buffer) + size, GFP_KERNEL);
	if (!buffer)
	{
		*retval = -ENOMEM;
		printk(KERN_NOTICE "minipipe:kmalloc failed\n");
		return NULL;
	}
	*retval = kfifo_init(&buffer->fifo, buffer + 1, size);
	if (*retval)
	{
		kfree(buffer);
		printk(KERN_NOTICE "minipipe:kfifo_init failed\n");
		return NULL;
	}
	mutex_init(&buffer->lock);
	*retval = 0;

	return buffer;
}

static void buffer_delete(struct buffer *buffer)
{
	kfree(buffer);
}

/* Create and initialize a single minipipe_dev */
static struct minipipe_dev *minipipe_create(dev_t dev_no,
											struct file_operations *fops, struct buffer *buffer, int *retval)
{
	struct minipipe_dev *minipipe = kmalloc(sizeof(struct minipipe_dev), GFP_KERNEL);
	if (!minipipe)
	{
		*retval = -ENOMEM;
		printk(KERN_NOTICE "minipipe:kmalloc failed\n");
		return NULL;
	}
	memset(minipipe, 0, sizeof(struct minipipe_dev));
	minipipe->buffer = buffer;

	minipipe->number_of_current_threads = 0;

	cdev_init(&minipipe->cdev, fops);
	minipipe->cdev.owner = THIS_MODULE;
	minipipe->cdev.ops = fops;
	*retval = cdev_add(&minipipe->cdev, dev_no, 1);
	minipipe->dev_no = dev_no;
	if (*retval)
	{
		printk(KERN_NOTICE "Error (%d) when adding device minipipe\n",
			   *retval);
		kfree(minipipe);
		minipipe = NULL;
	}

	return minipipe;
}
static void minipipe_delete(struct minipipe_dev *minipipe)
{
	cdev_del(&minipipe->cdev);
	kfree(minipipe);
}

/* Called when a process calls "open" on this device */
static int minipipe_open(struct inode *inode, struct file *filp)
{
	struct minipipe_dev *minipipe; /* device information */

	minipipe = container_of(inode->i_cdev, struct minipipe_dev, cdev);
	filp->private_data = minipipe; /* for other methods */

	int current_number_of_threads = minipipe->number_of_current_threads;
	if (current_number_of_threads + 1 == max_threads)
	{
		return -1;
	}

	return 0;
}

/* Called when a process performs "close" operation */
static int minipipe_release(struct inode *inode, struct file *filp)
{
	return 0; /* nothing to do; could not set this function in fops */
}

/* Read count bytes from buffer to user space ubuf */
static ssize_t minipipe_read(struct file *filp, char __user *ubuf, size_t count,
							 loff_t *f_pos /* ignoring f_pos */)
{
	ssize_t retval = 0;
	struct minipipe_dev *minipipe = filp->private_data;
	struct buffer *buffer = minipipe->buffer;
	struct kfifo *fifo = &buffer->fifo;
	unsigned int copied;

	if (mutex_lock_interruptible(&buffer->lock))
	{
		return -ERESTARTSYS;
	}

	dump_buffer(buffer);

	retval = kfifo_to_user(fifo, (char __user *)ubuf, count, &copied);
	if (retval)
	{
		printk(KERN_NOTICE "minipipe:kfifo_to_user failed\n");
	}
	else
	{
		retval = copied;
	}

	dump_buffer(buffer);

	mutex_unlock(&buffer->lock);

	return retval;
}

/* Write count bytes from user space ubuf to buffer */
static ssize_t minipipe_write(struct file *filp, const char __user *ubuf,
							  size_t count, loff_t *f_pos /* ignoring f_pos */)
{
	ssize_t retval = 0;
	struct minipipe_dev *minipipe = filp->private_data;
	struct buffer *buffer = minipipe->buffer;
	struct kfifo *fifo = &buffer->fifo;
	unsigned int copied;

	if (mutex_lock_interruptible(&buffer->lock))
	{
		return -ERESTARTSYS;
	}

	dump_buffer(buffer);

	retval = kfifo_from_user(fifo, (char __user *)ubuf, count, &copied);
	if (retval)
	{
		printk(KERN_NOTICE "minipipe:kfifo_from_user failed\n");
	}
	else
	{
		retval = copied;
	}

	dump_buffer(buffer);

	mutex_unlock(&buffer->lock);

	return retval;
}

static void dump_buffer(struct buffer *b)
{
	char buf[BUFFER_SIZE];
	size_t copied;

	memset(buf, 0, BUFFER_SIZE);
	copied = kfifo_out_peek(&b->fifo, buf, BUFFER_SIZE);

	printk(KERN_NOTICE "minipipe:buffer:size=%u:contains=%u:buf=%s\n",
		   kfifo_size(&b->fifo), kfifo_len(&b->fifo), buf);
}

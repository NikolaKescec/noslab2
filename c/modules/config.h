/*
 *
 * Copyright (C) 2021 Nikola Kescec
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form.
 * No warranty is attached.
 *
 */

#pragma once

#define DRIVER_NAME 	"minipipe"

#define AUTHOR		"Nikola Kescec"
#define LICENSE		"Dual BSD/GPL"

#define BUFFER_SIZE	64
#define MAX_THREADS 6

/* Circular buffer */
struct buffer {
	struct kfifo fifo;
	struct mutex lock;	/* prevent parallel access */
};

/* Device driver */
struct minipipe_dev {
	dev_t dev_no;		/* device number */
	struct cdev cdev;	/* Char device structure */
	struct buffer *buffer;	/* Pointer to buffer */
	int number_of_current_threads;
};

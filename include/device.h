#ifndef __DEVICE_H__
#define __DEVICE_H__

typedef int     dev_t;

struct device
{
    dev_t dev;
	const char * name;
	struct file_operations * fops;

    struct device *next;
    struct device *prev;
};

#define MAX_CHRDEV 32
#define MAX_BLKDEV 32

extern struct device *alloc_device_struct(void);
int put_device_struct(struct device *dev);

#endif 


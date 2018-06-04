#include "fs.h"
#include "chr.h"
#include "error.h"
#include "device.h"
#include "common.h"

static struct chr_dev_struct chr_devs[NR_CHRDEV];

struct chr_dev_struct *alloc_chr_dev_struct()
{
    int chr;

    for (chr = 0; chr < NR_CHRDEV; chr++) 
    {
        if (!chr_devs[chr].device)
            return &chr_devs[chr];
    }
    return NULL;
}

struct chr_dev_struct *find_chr_dev(dev_t major)
{
    int dev;
    for (dev = 0; dev < NR_CHRDEV; dev++)
    {
        if (chr_devs[dev].device && (chr_devs[dev].device->dev == major))
            return &chr_devs[dev];
    }

    return NULL;
}

struct file_operations * get_chrfops(unsigned int major)
{
    struct chr_dev_struct *chr_dev;

    if (major >= NR_CHRDEV)
		return NULL;

    chr_dev = find_chr_dev(major);
    if (!chr_dev || !chr_dev->device)
        return NULL;

    return chr_dev->device->fops;
}

int register_chrdev(unsigned int major, const char * name, struct file_operations *fops)
{
    struct chr_dev_struct *chr_dev;
    struct device *dev;


	if (major >= NR_CHRDEV)
		return -EINVAL;

    dev = alloc_device_struct();
    if (!dev)
        return -ENOMEM;

    chr_dev = alloc_chr_dev_struct();
    if (!chr_dev)
    {
        put_device_struct(dev);
        return -ENOMEM;
    }
    
    chr_dev->device = dev;
    dev->name = name;
    dev->dev  = major;
    dev->fops = fops;

	return 0;
}

int chrdev_open(struct inode *inode, struct file *filp)
{
    struct chr_dev_struct *chr_dev;

    chr_dev = find_chr_dev(inode->i_dev);

    if (!chr_dev)
		return -ENODEV;

	filp->f_op = chr_dev->device->fops;
	if (filp->f_op->open)
		return filp->f_op->open(inode,filp);
	return 0;

}

struct file_operations def_chr_fops = 
{
	chrdev_open,	/* open */
	NULL,		    /* close */
	NULL,		    /* read */
	NULL,		    /* write */
	NULL,	    	/* ioctl */
	NULL,	    	/* lseek */
	NULL,		    /* readdir */
};

struct inode_operations chrdev_inode_operations = 
{
	&def_chr_fops,		/* default file operations */
	NULL,			/* create */
	NULL,			/* lookup */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
};


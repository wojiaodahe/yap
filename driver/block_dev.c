#include "fs.h"
#include "blk.h"
#include "device.h"
#include "error.h"
#include "kmalloc.h"
#include "common.h"
#include "lib.h"

#define SYSTEM_DEFAULT_SECTOR_SIZE		512

//static struct blk_dev_struct *blk_devs_head;

struct blk_dev_struct blk_devs[NR_BLKDEV];
struct super_block super_blocks[NR_SUPER];

struct super_block *find_super(unsigned int dev)
{
	int sb;
    if (!dev)
        return NULL;
	
	for (sb = 0; sb < NR_SUPER; sb++)
    {
		if (super_blocks[sb].s_dev == dev)
			return &super_blocks[sb];
    }

    return NULL;
}

struct super_block *alloc_super()
{
    struct super_block *s;
    static int sb = 0;

    s = super_blocks;
    for (sb = 0; sb < NR_SUPER; sb++)
    {
        if (!s->s_dev)
            return s;
        s++;
    }

    return NULL;
}

struct blk_dev_struct *alloc_blk_dev_struct()
{
    int blk;

    for (blk = 0; blk < NR_BLKDEV; blk++) 
    {
        if (!blk_devs[blk].fops)
            return &blk_devs[blk];
    }
    return NULL;
}

struct blk_dev_struct *find_blk_dev(dev_t major)
{
    int dev;
    for (dev = 0; dev < NR_BLKDEV; dev++)
    {
        if (blk_devs[dev].dev == major)
            return &blk_devs[dev];
    }

    return NULL;
}

int register_get_disk_info_fun(dev_t major, get_disk_info get_info)
{
    struct blk_dev_struct *blk_dev;

    blk_dev = find_blk_dev(major);
	if (!blk_dev)
		return -EINVAL;
	
	blk_dev->get_disk_info = get_info;
	
	return 0;
}

int block_read(struct inode *inode, unsigned int pos, char *buf, int count)
{
    int ret;
    struct request *req;
    struct blk_dev_struct *blk_dev;
	struct disk_info *info;
	char *tmp_buff;

	unsigned int total_size;
	unsigned rw_sectors;

    if (!inode)
        return -EINVAL;

    blk_dev = find_blk_dev(inode->i_dev);
    if (!blk_dev)
        return -EINVAL;
	if (!blk_dev->get_disk_info)
		return -ENODEV;
    
    req = kmalloc(sizeof (struct request));
	if (!req)
		return -ENOMEM;
	info = kmalloc(sizeof (struct disk_info));
	if (!info)
	{
		kfree(req);
		return -ENOMEM;
	}

	blk_dev->get_disk_info(info);

	total_size = count * SYSTEM_DEFAULT_SECTOR_SIZE;
	rw_sectors = total_size / info->sector_size;
	if (total_size % info->sector_size)
		rw_sectors += 1;
#if 1
	if ((rw_sectors * info->sector_size - (pos * SYSTEM_DEFAULT_SECTOR_SIZE) % info->sector_size) < total_size)
		rw_sectors += 1;
#else
	
#endif
	tmp_buff = kmalloc(rw_sectors * info->sector_size);
	if (!tmp_buff)
	{
		kfree(info);
		kfree(req);
		return -ENOMEM;
	}

    req->sector = pos * SYSTEM_DEFAULT_SECTOR_SIZE / info->sector_size;
    req->nr_sectors = rw_sectors;
    req->buffer = tmp_buff;
    req->inode = inode;
    req->cmd = REQUEST_READ;
    ret = blk_dev->request_fn(req);

	memcpy(buf, tmp_buff + (SYSTEM_DEFAULT_SECTOR_SIZE * pos % info->sector_size), count * SYSTEM_DEFAULT_SECTOR_SIZE);

	kfree(tmp_buff);
	kfree(info);
    kfree(req);
    
    return ret;
}

int block_write(struct inode * inode, unsigned int pos, char * buf, int count)
{
    int ret;
    struct request *req;
    struct blk_dev_struct *blk_dev;
    
    if (!inode)
        return -EINVAL;

    blk_dev = find_blk_dev(inode->i_dev);
    if (!blk_dev)
        return -EINVAL;

    req = kmalloc(sizeof (struct request));

    req->sector = pos;
    req->nr_sectors = count;
    req->buffer = buf;
    req->inode = inode;
    req->cmd = REQUEST_WRITE;
    ret = blk_dev->request_fn(req);

    kfree(req);
    
	return ret;
}

struct file_operations *get_blkfops(unsigned int major)
{
    struct blk_dev_struct *blk_dev;
	
    if (major >= NR_BLKDEV)
		return NULL;
    
    blk_dev = find_blk_dev(major);
    if (!blk_dev)
        return NULL;

    return blk_dev->fops;
}

int register_blkdev(unsigned int major, char * name, struct file_operations *fops)
{
    struct blk_dev_struct *blk_dev;

    if (major >= MAX_BLKDEV)
		return -EINVAL;

    blk_dev = alloc_blk_dev_struct();
    if (!blk_dev)
        return -ENOMEM;

    blk_dev->name = name;
    blk_dev->dev  = major;
    blk_dev->fops = fops;

	return 0;
}

int unregister_blkdev(unsigned int major)
{
	//!!!!!!!!!
	//!!!!!!!!
	//!!!!!!!!!
	//
    // check dev 
	// if (dev xxxx)
	// return -EINVAL;
    return 0;
}

int register_blk_request(int dev, int (*req)(struct request*))
{
    struct blk_dev_struct *blk_dev;

    if (dev >= MAX_BLKDEV)
		return -EINVAL;

    blk_dev = find_blk_dev(dev);
    if (!blk_dev)
        return -ENODEV;
    
    if (blk_dev->request_fn)
        return -EBUSY;

    blk_dev->request_fn = req;

	return 0;
}

int blkdev_open(struct inode * inode, struct file * filp)
{
    
    struct blk_dev_struct *blk_dev;

    blk_dev = find_blk_dev(inode->i_dev);

    if (!blk_dev)
		return -ENODEV;

	filp->f_op = blk_dev->fops;
	if (filp->f_op->open)
		return filp->f_op->open(inode,filp);
	return 0;
}	

/*
 * Dummy default file-operations: the only thing this does
 * is contain the open that then fills in the correct operations
 * depending on the special file...
 */
struct file_operations def_blk_fops = 
{
	blkdev_open,	/* open */
	NULL,		    /* close */
	NULL,		    /* read */
	NULL,		    /* write */
	NULL,	    	/* ioctl */
	NULL,	    	/* lseek */
	NULL,		    /* readdir */

};

struct inode_operations blkdev_inode_operations = 
{
	&def_blk_fops,		/* default file operations */
	NULL,			/* create */
	NULL,			/* lookup */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
};




#include "fs.h"
#include "error.h"
#include "blk.h"
#include "ramfs.h"
#include "common.h"
#include "lib.h"
#include "printk.h"

#define RAMDISK_SIZE    (1024 *1024 *2)
#define RAMDISK_DEV_NUM     1
extern int ROOT_DEV;
extern struct inode_operations ramfs_inode_operations;

int ramdisk_do_request(struct request *req)
{
    unsigned char *start;
    int size;
    struct ramfs_inode *ramfs_inode;

    printk("ramdisk request\r\n"); 
    if (req->inode == NULL)
        return -EINVAL;

    ramfs_inode = req->inode->i_what;

    if (!ramfs_inode)
        return -EINVAL;

    start = ramfs_inode->start_addr;
    if (!start)
        return 0;
    
    start += req->sector;    
    if (start >= ramfs_inode->start_addr + ramfs_inode->size)
        return 0;

    if ((start + req->nr_sectors) > (ramfs_inode->start_addr + ramfs_inode->size))
        size = ramfs_inode->start_addr + ramfs_inode->size - start;
    else
        size = req->nr_sectors;
    
    if (req->cmd == REQUEST_READ)
        memcpy(req->buffer, start, size);
    else
        memcpy(start, req->buffer, size);
    return size;
}

int ramdisk_get_disk_info(struct disk_info *info)
{
	if (!info)
		return -EINVAL;
	
	info->sector_size = 512;
	return 0;
}

//!!!!!!!!!!!!!
//!!!!!!!!!!!!!
//!!!!!!!!!!!!!
//WARNING! MAYBE BUGS!
int ramdisk_init(void)
{
	int ret = 0;

    //ROOT_DEV = RAMDISK_DEV_NUM;
	

    ret = register_blkdev(RAMDISK_DEV_NUM, "ramdisk", ramfs_inode_operations.default_file_ops);
	if (ret)
	{
		//need release something?
		return ret;
	}

    ret = register_get_disk_info_fun(RAMDISK_DEV_NUM, ramdisk_get_disk_info);
	if (ret)
	{
		//need release something?
		return ret;
	}


	//if error, need release something?
   	return register_blk_request(RAMDISK_DEV_NUM, ramdisk_do_request);
}


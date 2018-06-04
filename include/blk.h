#ifndef __BLK_H__
#define __BLK_H__

#define REQUEST_READ    1
#define REQUEST_WRITE   2

#define NR_SUPER    MAX_BLKDEV
#define NR_BLKDEV   NR_SUPER

struct request 
{
	int dev;		/* -1 if no request */
	int cmd;		/* READ or WRITE */
	int errors;
	unsigned long sector;
	unsigned long nr_sectors;
	unsigned long current_nr_sectors;
    struct inode *inode;
	char * buffer;
//	struct task_struct * waiting;
//	struct buffer_head * bh;
//	struct buffer_head * bhtail;
	struct request * next;
};

struct disk_info
{
	unsigned int sector_size;
};
typedef int (*get_disk_info)(struct disk_info *);

struct blk_dev_struct 
{
	int (*request_fn)(struct request *);
    struct super_block *super;
    struct device *device;
	get_disk_info get_disk_info;
};

extern int register_blk_request(int dev, int (*req)(struct request*));
int block_read(struct inode *inode, unsigned int pos, char *buf, int count);
int block_write(struct inode * inode, unsigned int pos, char * buf, int count);

#endif


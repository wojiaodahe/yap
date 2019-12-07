#include "ofs.h"
#include "fs.h"
#include "error.h"
#include "blk.h"
#include "kmalloc.h"
#include "common.h"
#include "printk.h"
#include "lib.h"

int ofs_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    return ret;
}

void ofs_close(struct inode *inode, struct file *filp)
{

}

int ofs_read(struct inode *inode, struct file *filp, char *buf, int len)
{
    int ret;
    char *buff;
    int block_size;
    int rw_block;
    int nr_blocks = 0;
    struct ofs_inode *o_inode;
    struct ofs_super_block *ofs_sb;

    if (!len)
        return len;
    
    if (!inode || !inode->i_sb)
        return -EINVAL;
    
    o_inode = inode->i_what;
    ofs_sb = inode->i_sb->sb_data;
    block_size = ofs_sb->block_size;
    
    rw_block = filp->f_pos / block_size + o_inode->start_block;

    if ((filp->f_pos + len) > o_inode->size)
        len = o_inode->size - filp->f_pos;

    nr_blocks += len / block_size;
    if (len % block_size)
        nr_blocks++;
#if 0 
	if (filp->f_pos % block_size)
        nr_blocks++;
#else
	if ((nr_blocks * block_size - (filp->f_pos % block_size)) < len)
		nr_blocks++;
#endif

    buff = kmalloc(nr_blocks * block_size);
#if 0
    printk("\nfilp->f_pos: %d\n", filp->f_pos);
    printk("len: %d\n", len);
    printk("nr_blocks: %d\n", nr_blocks);
    printk("rw_block: %d\n", rw_block);
    printk("filp offset: %d\n", filp->f_pos % block_size);
#endif
    ret = block_read(inode, rw_block, buff, nr_blocks);
    if (ret < 0)
    {   
        kfree(buff);
        return ret;
    }

    memcpy(buf, buff + filp->f_pos % block_size, len);
    kfree(buff);

    filp->f_pos += len;

    return len;
}

int ofs_write(struct inode *inode, struct file *filp, char *buf, int len)
{
    int ret;
    char *buff;
    int block_size;
    int rw_block;
    int nr_blocks = 0;
    struct ofs_inode *o_inode;
    struct ofs_super_block *ofs_sb;

    if (!len)
        return len;
    
    if (!inode || !inode->i_sb)
        return -EINVAL;
    
    o_inode = inode->i_what;
    ofs_sb = inode->i_sb->sb_data;
    block_size = ofs_sb->block_size;
    
    rw_block = filp->f_pos / block_size + o_inode->start_block;

    if ((filp->f_pos + len) > o_inode->size)
        len = o_inode->size - filp->f_pos;

    nr_blocks += len / block_size;
    if (len % block_size)
        nr_blocks++;
    if (filp->f_pos % block_size)
        nr_blocks++;

    buff = kmalloc(nr_blocks * block_size);
    
    printk("\nfilp->f_pos: %d\n", filp->f_pos);
    printk("len: %d\n", len);
    printk("nr_blocks: %d\n", nr_blocks);
    printk("rw_block: %d\n", rw_block);
    printk("filp offset: %d\n", filp->f_pos % block_size);
    
    ret = block_read(inode, rw_block, buff, nr_blocks);
    if (ret < 0)
        return ret;

    memcpy(buff + filp->f_pos % block_size, buf, len);
    ret = block_write(inode, rw_block, buff, nr_blocks);
    kfree(buff);
    if (ret < 0)
    {
        return ret;
    }
    
    filp->f_pos += len;

    return len;
}

int ofs_lookup(struct inode *dir, char *name, int namelen, struct inode **res_inode)
{
    struct ofs_inode *o_inode;
   
    o_inode = dir->i_what;
    if (!o_inode)
        return -EINVAL;
    
    if (strcmp(o_inode->name, name) == 0)
    {
        *res_inode = o_inode->inode;
         return 0;
    }
    else 
        return -EINVAL;
}

int ofs_create(struct inode *dir, char *name, int namelen, int mode, struct inode **res_inode)
{
    return 0;
}

extern struct inode_operations chrdev_inode_operations;
extern struct inode_operations blkdev_inode_operations;

int ofs_mknod(struct inode *dir, char *name, int namelen, int mode, int dev_num)
{
    return 0;
}

struct file_operations ofs_file_operations = 
{
    ofs_open,
    ofs_close,
    ofs_read,
    ofs_write,
    NULL,
    NULL,
    NULL,
};

struct inode_operations ofs_inode_operations = 
{
    &ofs_file_operations, 
    ofs_create,
    ofs_lookup,
    NULL,
    NULL,
    ofs_mknod,
    NULL,
};


struct super_block *ofs_read_super(struct super_block *sb)
{
    int ret;
    char *buff;
    int disk_secotr_size;
    struct inode *inode;
    struct ofs_inode *ofs_inode;
    struct ofs_super_block *ofs_sb;

    inode = kmalloc(sizeof (struct inode));
    if (!inode)
        return NULL;
    
    ofs_inode = kmalloc(sizeof (struct ofs_inode));
    if (!ofs_inode)
    {
        kfree(inode);
        return NULL;
    }
    
    ofs_sb = kmalloc(sizeof (struct ofs_super_block));
    if (!ofs_sb)
    {
        kfree(inode);
        kfree(ofs_inode);
        return NULL;
    }
   
    inode->i_dev = sb->s_dev;

//    disk_secotr_size = get_disk_sector_size();

    disk_secotr_size = 512;
    buff = kmalloc(disk_secotr_size);
    if (!buff)
    {
        kfree(inode);
        kfree(ofs_sb);
        kfree(ofs_inode);
        return NULL;
    }

    ret = block_read(inode, 0, buff, 1);
    
    if (ret < 0)
    {
        kfree(buff);
        kfree(inode);
        kfree(ofs_sb);
        kfree(ofs_inode);
        return NULL;
    }
    
    memcpy(ofs_sb, buff, sizeof (struct ofs_super_block));
    memcpy(ofs_inode, buff + sizeof (struct ofs_super_block), sizeof (struct ofs_inode));

    inode->i_op = &ofs_inode_operations;
    inode->i_what = ofs_inode;
    ofs_inode->inode = inode;
    inode->i_sb = sb;
    
    sb->s_type = FILE_SYSTEM_TYPE_OFS;
    sb->s_mounted = inode;
    sb->sb_data = ofs_sb;
    kfree(buff);

    return sb;
}


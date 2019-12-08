#include "fs.h"
#include "error.h"
#include "ramfs.h"
#include "blk.h"
#include "kmalloc.h"
#include "common.h"
#include "lib.h"


int  ramfs_open   (struct inode *, struct file *);
void ramfs_close (struct inode *, struct file *);
int ramfs_read(struct inode *inode, struct file *filp, char *buf, int len);
int ramfs_write(struct inode *inode, struct file *filp, char *buf, int len);
int ramfs_lookup(struct inode *dir, char *name, int namelen, struct inode **res_inode);
int ramfs_create(struct inode *dir, char *name, int namelen, int mode, struct inode **res_inode);
int ramfs_mknod(struct inode *dir, char *name, int namelen, int mode, int dev_num);
int ramfs_mkdir(struct inode *dir, char *name, int namelen, int mode);

struct file_operations ramfs_file_operations = 
{
    ramfs_open,
    ramfs_close,
    ramfs_read,
    ramfs_write,
    NULL,
    NULL,
    NULL,
};

struct inode_operations ramfs_inode_operations = 
{
    &ramfs_file_operations, 
    ramfs_create,
    ramfs_lookup,
    ramfs_mkdir,
    NULL,
    ramfs_mknod,
    NULL,
};


struct ramfs_inode *ramfs_init()
{
	return NULL;
}

int ramfs_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    return ret;
}

void ramfs_close(struct inode *inode, struct file *filp)
{

}

int ramfs_read(struct inode *inode, struct file *filp, char *buf, int len)
{
    int ret;

    ret = block_read(inode, filp->f_pos, buf, len);

    if (ret < 0)
        return ret;

    filp->f_pos += ret;

    return ret;
}

int ramfs_write(struct inode *inode, struct file *filp, char *buf, int len)
{
    int ret;

    ret = block_write(inode, filp->f_pos, buf, len);

    if (ret < 0)
        return ret;

    filp->f_pos += ret;

    return ret;
}

struct ramfs_inode *ramfs_travel_child(struct ramfs_inode *parent,   char *name, int namelen)
{
    struct ramfs_inode *child;
    if (!parent)
        return NULL;
    
    child = parent->child;

    if (!child)
        return NULL;
        
    if (strncmp(child->name, name, namelen) == 0)
            return child;

    while (child != NULL)
    {
        if (strncmp(child->name, name, namelen) == 0)
            return child;
        child = child->next;
    }

    return NULL;
}

int ramfs_lookup(struct inode *dir,   char *name, int namelen, struct inode **res_inode)
{
    struct ramfs_inode *parent;
    struct ramfs_inode *child;

    parent = dir->i_what;

    child = ramfs_travel_child(parent, name, namelen);

    if (!child)
        return -1;

    *res_inode = child->inode;
    
    return 0;
}

int ramfs_add_node(struct ramfs_inode *parent, struct ramfs_inode *child)
{
   struct ramfs_inode *tmp; 

   tmp = parent->child;

   if (tmp == NULL)
   {
       parent->child = child;
       return 0;
   }
   while (tmp->next != NULL)
       tmp = tmp->next;

   tmp->next = child;
   child->prev = tmp;
   child->next = NULL;

   return 0;
}

int ramfs_create(struct inode *dir, char *name, int namelen, int mode, struct inode **res_inode)
{
    struct ramfs_inode *parent;
    struct ramfs_inode *child;
    struct inode       *inode;

    parent = dir->i_what;

    if (!parent)
        return -1;

    child = ramfs_travel_child(parent, name, namelen);
    if (child)
        return -1;

    child = kmalloc(sizeof (struct ramfs_inode));
    if (!child)
        return -1;

    inode = kmalloc(sizeof (struct inode));
    if (!inode)
        return -1;

    memcpy(child->name, name, namelen);
   
    child->inode = inode;
    inode->i_what = child;
	inode->i_dev = dir->i_dev;

    inode->i_op = &ramfs_inode_operations;

	*res_inode = inode;

    child->start_addr = kmalloc(RAMFS_DEFAULT_FILE_LEN);
    child->size = RAMFS_DEFAULT_FILE_LEN;

    return ramfs_add_node(parent, child);
}
extern struct inode_operations chrdev_inode_operations;
extern struct inode_operations blkdev_inode_operations;

int ramfs_mkdir(struct inode *dir,   char *name, int namelen, int mode)
{
    struct ramfs_inode *parent;
    struct ramfs_inode *child;
    struct inode       *inode;

    parent = dir->i_what;

    if (!parent)
        return -1;

    child = ramfs_travel_child(parent, name, namelen);
    if (child)
        return -1;

    child = kmalloc(sizeof (struct ramfs_inode));
    if (!child)
        return -1;

    inode = kmalloc(sizeof (struct inode));
    if (!inode)
        return -1;

    memcpy(child->name, name, namelen);
   
    child->inode = inode;
    inode->i_what = child;
	inode->i_dev = dir->i_dev;

    inode->i_op = &ramfs_inode_operations;

    mode &= ~(S_IFMT);
    mode |= S_IFDIR;

    inode->i_mode = mode;

    child->start_addr = kmalloc(RAMFS_DEFAULT_FILE_LEN);
    child->size = RAMFS_DEFAULT_FILE_LEN;

    return ramfs_add_node(parent, child);
}

int ramfs_mknod(struct inode *dir,   char *name, int namelen, int mode, int dev_num)
{
    struct ramfs_inode *parent;
    struct ramfs_inode *child;
    struct inode       *inode;
    
    if (!dir)
        return -ENOENT;

    parent = dir->i_what;

    if (!parent)
        return -ENOENT;

    child = ramfs_travel_child(parent, name, namelen);
    if (child)
        return -EBUSY;

    child = kmalloc(sizeof (struct ramfs_inode));
    if (!child)
        return -ENOSPC;
    memcpy(child->name, name, namelen);

    inode = kmalloc(sizeof (struct inode));
    if (!inode)
    {
        kfree(child);
        return -ENOSPC;
    }
	inode->i_mode = mode;
	inode->i_op = NULL;
	inode->i_dev = dev_num;
    inode->i_what = child;
    child->inode = inode;
   
    if (S_ISCHR(inode->i_mode)) 
        inode->i_op = &chrdev_inode_operations;
    else if (S_ISBLK(inode->i_mode))
        inode->i_op = &blkdev_inode_operations;
    
    return ramfs_add_node(parent, child);
}

struct super_block *ramfs_read_super(struct super_block *sb)
{
    struct inode *inode;
    struct ramfs_inode *ramfs_inode;

    inode = kmalloc(sizeof (struct inode));
    if (!inode)
        return NULL;
    
    ramfs_inode = kmalloc(sizeof (struct ramfs_inode));
    if (!ramfs_inode)
    {
        kfree(inode);
        return NULL;
    }

    sb->s_mounted = inode;
    sb->s_type = FILE_SYSTEM_TYPE_RAMFS;
    
    inode->i_dev = sb->s_dev;

    inode->i_op = &ramfs_inode_operations;
	inode->i_what = ramfs_inode;
    ramfs_inode->inode = inode;

    return sb;
}


#ifndef __RAMFS_H__
#define __RAMFS_H__

#define RAMFS_NAME_LEN          16
#define RAMFS_DEFAULT_FILE_LEN  512

struct ramfs_inode
{
    char name[RAMFS_NAME_LEN];
    unsigned char *start_addr;
    unsigned int size;

    struct ramfs_inode *parent;
    struct ramfs_inode *child;
    struct ramfs_inode *next;
    struct ramfs_inode *prev;
    struct inode       *inode;
};
extern struct super_block *ramfs_read_super(struct super_block *sb);
#endif 



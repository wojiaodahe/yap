#ifndef __OSF_H__
#define __OSF_H__

#define OFS_FILE_NAME_LEN       16
#define OFS_FILE_MAGIC_LIN      8
struct ofs_inode
{
    char name[OFS_FILE_NAME_LEN];
    int size;
    int start_block;
    struct inode *inode;
};

struct ofs_super_block
{
    unsigned char magic[OFS_FILE_MAGIC_LIN];
    unsigned int block_size;
    unsigned int version;
    unsigned int first_ofs_inode_offset;
};

#define FILE_SYSTEM_TYPE_OFS  2

extern struct super_block *ofs_read_super(struct super_block *sb);
#endif



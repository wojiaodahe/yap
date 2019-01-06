#include "ofs.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <string.h>

int main(int argc, const char *argv[])
{
    int ret;
    char *buff;
    int fd_src;
    int fd_dest;
    struct stat stbuf;
    char *fd_dest_name = "ofs.img";
    struct ofs_super_block *ofs_sb;
    struct ofs_inode *ofs_inode;
    int block_size = 512;
    int buff_size;

    if (argc < 2)
        printf("Usage: %s <file name>\n", argv[0]);

    if ((fd_src = open(argv[1], O_RDONLY)) < 0)
    {
        perror("open src");
        return fd_src;
    }

    if ((fd_dest = open(fd_dest_name, O_RDWR | O_CREAT, 0666)) < 0)
    {
        perror("open dest");
        close(fd_src);
        return fd_dest;
    }

    ret = fstat(fd_src, &stbuf);
    if (ret < 0)
    {
        close(fd_src);
        close(fd_dest);
        perror("fstat");
        return ret;
    }

    buff_size = stbuf.st_size + (block_size * 2);
    printf("file_size: %d buff_size: %d\n", stbuf.st_size, buff_size);

    buff = malloc(buff_size);
    if (!buff)
    {
        close(fd_src);
        close(fd_dest);
        perror("malloc");
        return ret;
    }

    ofs_sb = (struct ofs_super_block *)buff;
    memcpy(ofs_sb->magic, "OFS", 3);
    ofs_sb->block_size = block_size;
    ofs_sb->version = 1;
    ofs_sb->first_ofs_inode_offset = sizeof (struct ofs_super_block);

    ofs_inode = (struct ofs_inode *)(buff + sizeof (struct ofs_super_block));
    strcpy(ofs_inode->name, argv[1]);
    ofs_inode->start_block = 1;
    ofs_inode->size = stbuf.st_size;
    
    if ((ret = read(fd_src, buff + block_size, stbuf.st_size)) < 0)
    {
        close(fd_src);
        close(fd_dest);
        perror("read");
        return 0;
    }

    printf("read len: %d\n", ret);
    if ((ret = write(fd_dest, buff, buff_size)) < 0)
        perror("write");
    printf("write len: %d\n", ret);

    close(fd_src);
    close(fd_dest);

    return 0;
}

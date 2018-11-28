#ifndef __VFS_H__
#define __VFS_H__


extern int sys_open(char *name, unsigned int flag, int mode);
extern int sys_close(unsigned int fd);
extern int sys_read(unsigned int fd, char *buf, unsigned int count);
extern int sys_write(unsigned int fd, char *buf, unsigned int count);
extern int sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
extern int sys_mkdir(char * pathname, int mode);
extern int sys_mknod(char * filename, int mode, unsigned int dev);
extern int vfs_init(void);

#endif


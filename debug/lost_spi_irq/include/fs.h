#ifndef __FS_H__
#define __FS_H__

#define O_RDONLY	    00
#define O_WRONLY	    01
#define O_RDWR		    02
#define O_CREAT		  0100	/* not fcntl */
#define O_EXCL		  0200	/* not fcntl */
#define O_NOCTTY	  0400	/* not fcntl */
#define O_TRUNC		 01000	/* not fcntl */
#define O_APPEND	 02000
#define O_NONBLOCK	 04000

#define FILE_SYSTEM_TYPE_RAMFS      1

struct super_block
{

    struct inode *s_covered;
    struct inode *s_mounted;
    unsigned long s_dev;
    unsigned int  s_flags;
    unsigned int  s_type;
    int start_block;
    void *sb_data;
};


struct file_system_type 
{
	struct super_block *(*read_super) (struct super_block *);
	char *name;
	int requires_dev;
};

#define NAME_MAX 16
struct dirent 
{
	unsigned short	d_reclen;
    unsigned int	d_off;
	long		    d_ino;
	char		    d_name[NAME_MAX + 1];
};

struct inode 
{
	int 		            i_dev;
	unsigned long	        i_ino;
	struct inode_operations *i_op;
	struct super_block      *i_sb;
	struct inode            *i_next;
    struct inode            *i_prev;
	struct inode            *i_mount;
    unsigned int            i_mode;
    unsigned int            i_rdev;
	unsigned short          i_count;
	unsigned short          i_flags;
	unsigned int 			i_socket;
    void                    *i_what;
};

struct file 
{
    unsigned int            f_mode;
    unsigned int            f_dev;
    unsigned int            f_pos;
    unsigned short          f_flags;
    unsigned short          f_count;
    unsigned short          f_reada;
    
    struct file             *f_next;
    struct file             *f_prev;
    struct inode            *f_inode;
    struct file_operations *f_op;
    void *private_data;
};


struct file_operations 
{
	int (*open)   (struct inode *, struct file *);
	void (*close) (struct inode *, struct file *);
	int (*read)   (struct inode *, struct file *, char *, int);
	int (*write)  (struct inode *, struct file *, char *, int);
	int (*ioctl)  (struct inode *, struct file *, unsigned int, unsigned long);
	int (*lseek)  (struct inode *, struct file *, unsigned int, int);
	int (*readdir)(struct inode *, struct file *, struct dirent *, int);
	void (*release) (struct inode *, struct file *);
};

struct inode_operations 
{
	struct file_operations *default_file_ops;
	int (*create)     (struct inode *, char *,int,int,struct inode **);
	int (*lookup)     (struct inode *, char *,int,struct inode **);
	int (*mkdir)      (struct inode *, char *,int,int);
	int (*rmdir)      (struct inode *, char *,int);
	int (*mknod)      (struct inode *, char *,int,int,int);
	int (*rename)     (struct inode *, char *,int,struct inode *,const char *,int);
};

#define MAJOR(a) (((unsigned int)(a)) >> 8)
#define MINOR(a) ((a) & 0xff)

#define S_IFMT  00170000
#define S_IFSOCK 0140000
#define S_IFLNK	 0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define NR_OPEN    	16
#define NR_FILEP	32

extern int sys_mknod(char * filename, int mode, unsigned int dev);
extern int sys_read(unsigned int fd, char *buf, unsigned int count);
extern int sys_write(unsigned int fd, char *buf, unsigned int count);
extern int sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);

#endif



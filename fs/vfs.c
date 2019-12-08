#include "error.h"
#include "fs.h"
#include "kmalloc.h"
#include "common.h"
#include "pcb.h"
#include "blk.h"
#include "ramfs.h"
#include "ofs.h"
#include "proc.h"
#include "lib.h"
#include "printk.h"

extern int nand_init(void);
extern int ramdisk_init(void);
extern int lookup(struct inode *dir, char *name, int len, struct inode **result);
extern int dir_namei(char *pathname, int *namelen, char **name, struct inode *base, struct inode **res_inode);
extern pcb_t *current;

static struct file FILP[NR_FILEP];
int ROOT_DEV = 0;

int open_namei(char *pathname, int flag, int mode, struct inode **res_inode, struct inode *base)
{
    int ret = 0;
    char *basename;
    int namelen;
    struct inode *dir;
    struct inode *inode;

    ret = dir_namei(pathname, &namelen, &basename, base, &dir);

    if (ret)
        return ret;
    
    if (!namelen)
    {
        //check permission

        *res_inode = dir;
        return 0;
    }

    dir->i_count++;
    
    if (flag & O_CREAT)
    {
        ret = lookup(dir, basename, namelen, &inode);
        if (!ret)
        {
   //         if (flag & O_EXECL)
    //            ret = -EEXIST;
        }
        else
        {
            dir->i_count++;
            ret = dir->i_op->create(dir, basename, namelen, mode, res_inode);
            return ret;
        }
#if 0
        else if (!permission)
            ret = -EACCES;
        else if (!dir_i_op || !dir->i_op->create)
            ret = -EACCES;
        else if (IS_RDONLY(dir))
            ret = -EROFS;
        else
        {
            dir->i_count++;
            ret = dir->i_op->create(dir, basename, namelen, mode, res_inode);
            return ret;
        }
#endif 
    }
    else 
        ret = lookup(dir, basename, namelen, &inode);
    if (ret)
        return ret;

    *res_inode = inode;
    return ret;
}

static struct file_system_type file_systems[] =
{
	{ramfs_read_super,	"ramfsfs", 1},
    {ofs_read_super,    "ofs",     3},
	{NULL,			NULL,		0}
};


struct file_system_type *get_fs_type(char *name)
{
    int i;

    for (i = 0; file_systems[i].read_super; i++)
    {
        if (!strcmp(name, file_systems[i].name))
            return (&file_systems[i]);
    }
    return NULL;
}

int fs_may_mount(unsigned int dev)
{
    return 1;
}

extern struct super_block *find_super(unsigned int dev);
extern struct super_block *alloc_super(void);
struct super_block *read_super(unsigned int dev, char *name, int flags, void *data)
{
    struct super_block *sb;
    struct file_system_type *fstype;

    sb = find_super(dev);

    if (sb)
        return sb;
    
	if (!(fstype = get_fs_type(name)))
    {
        printk("VFS: get_fs_type");
        return NULL;
    }
    
    sb = alloc_super();
    if (!sb)
        return NULL;

    sb->s_dev = dev;
    sb->s_flags = flags;
    
    if (!(fstype->read_super(sb)))
    {
        sb->s_dev = 0;
        return NULL;
    }
    
    sb->s_covered = NULL;
    
    return sb;
}

int lookup(struct inode *dir, char *name, int len, struct inode **result)
{
    struct super_block *sb;

    *result = NULL;
    if (!dir)
        return -ENOTDIR;

    //chenck permission 
    
    if (len == 2 && name[0] == '.' && name[1] == '.')
    {
        if (dir == current->root)
        {
            *result = dir;
            return 0;
        }
        else if ((sb = dir->i_sb) && (dir = sb->s_mounted))
        {
            sb = dir->i_sb;
            dir = sb->s_covered;
            if (!dir)
                return -ENOENT;
            dir->i_count++;
        }
    }
    if (!dir->i_op || !dir->i_op->lookup)
        return -ENOTDIR;

    // if (!permission)
    //      return -EACCES;
    if (!len)
    {
        *result = dir;
        return 0;
    }

    return dir->i_op->lookup(dir, name, len, result);
}

int dir_namei(char *pathname, int *namelen, char **name, struct inode *base, struct inode **res_inode)
{
    char c;
    char *thisname;
    int len;
    int ret;
    struct inode *inode;

    *res_inode = NULL;
    if (!base)
    {
        base = current->pwd;
        base->i_count++;
    }
    if ((c = *pathname) == '/')
    {
        base = current->root;
        pathname++;
        base->i_count++;
    }

    while (1)
    {
        thisname = pathname;
        
        for (len = 0; (c = *(pathname++)) && (c != '/'); len++)
            ;
    
        if (!c)
            break;

        ret = lookup(base, thisname, len, &inode);
        if (ret)
            return ret;
        
        base = inode;
        if (base->i_mount)
            base = base->i_mount;
        base->i_count++;
    }
    if (!base->i_op || !base->i_op->lookup)
        return -ENOTDIR;

    *name = thisname;
    *namelen = len;
    *res_inode = base;
    
    return 0;
}

int _namei(char *pathname, struct inode *base, struct inode **res_inode)
{
    char *basename;
    int namelen;
    int ret;
    struct inode *inode;

    *res_inode = NULL;
    ret = dir_namei(pathname, &namelen, &basename, base, &base);
    if (ret)
        return ret;

    base->i_count++;
    ret = lookup(base, basename, namelen, &inode);
    if (ret)
        return ret;

    inode->i_count++;
    *res_inode = inode;
    
    return 0;
}

int namei(char *pathname, struct inode **inode)
{
    int ret;
    
    ret = _namei(pathname, NULL, inode);

    return ret;
}

int do_mount(unsigned int dev, char *dir, char *type, int flags)
{
    struct inode *dir_i;
    struct super_block *sb;
    int ret;

    ret = namei(dir, &dir_i);
    if (ret)
        return ret;

    if (dir_i->i_count != 1 || dir_i->i_mount)
        return -EBUSY;

    if (!S_ISDIR(dir_i->i_mode))
        return -EPERM;

    if (!fs_may_mount(dev))
        return -EBUSY;

    sb = read_super(dev, type, flags, 0);
    if (!sb || sb->s_covered)
        return -EBUSY;
    sb->s_covered = dir_i;
    dir_i->i_mount = sb->s_mounted;

    return 0;
}

void mount_root()
{
    struct file_system_type *fstype;
    struct super_block *sb;
    struct inode *inode;

    for (fstype = file_systems; fstype->read_super; fstype++)
    {
        sb = read_super(ROOT_DEV, fstype->name, 0, NULL);
        if (sb)
        {
            inode = sb->s_mounted;
            inode->i_count += 3;
            sb->s_covered= inode;
            sb->s_flags = 0;
            current->root = inode;
            current->pwd = inode;
            return;
        }
    }

    printk("VFS; Unable to mount root\n");
    while (1)
        ;
}

int sys_mount(char *dev_name, char *dir_name, char *type, unsigned long flags, void *data)
{
   struct inode *inode;
   struct file_operations *fops;
   struct file_system_type *fstype;
   unsigned int dev;
   int ret;
   char *t;
   
   //check flags

   fstype = get_fs_type(type);

   if (!fstype)
       return -ENODEV;

   t = fstype->name;

   if (fstype->requires_dev)
   {
       ret = namei(dev_name, &inode);
       if (ret)
           return ret;

       if (!S_ISBLK(inode->i_mode))
       {
           return -ENOTBLK;
       }
      // if (IS_NODEV(inode))
       {
        //   return -EACCES;
       }
       dev = inode->i_rdev;

       //check max_block_dev
   }
   else
   {
       //if (!(dev = get_unnamed_dev()))
       //    return -EMFILE;
   }

   fops = get_blkfops(MAJOR(dev));
   if (fops && fops->open)
   {
      ret = fops->open(inode, NULL)  ;
      if (ret)
          return ret;
   }

   ret = do_mount(dev, dir_name, t, flags);
   if (ret && fops && fops->close)
       fops->close(inode, NULL);

   return ret;
}

int sys_unmount(char *dir_name)
{

    //put_sb();
	return 0;
}

//!!!!!!!!!!!!!
//WARNING! MAYBE BUGS!
struct file *get_empty_filp()
{
	int i = 0;

	for (i = 0; i < NR_FILEP; i++)
	{
		if (!FILP[i].f_op)
			return &FILP[i];
	}

    return NULL;
}

struct inode *get_empty_inode()
{
	return kmalloc(sizeof (struct inode));
}

int sys_open(char *name, unsigned int flag, int mode)
{
    int fd;
    int ret = 0;
    struct file  *f = NULL;
    struct inode *file_inode = NULL;

    for (fd = 0; fd < NR_OPEN; fd++)
    {
		if (!current->filp[fd])
			break;
    }
    if (fd > NR_OPEN)
        return -EMFILE;

    f = get_empty_filp();
    if (!f)
        return -EMFILE;

    current->filp[fd] = f;

    f->f_flags = flag;
    f->f_mode = mode;
    f->f_count++;   

    ret = open_namei(name, flag, mode, &file_inode, NULL);
    if (ret || !file_inode)
    {
        current->filp[fd] = NULL;
        f->f_count--;
        return ret;
    }

    f->f_inode = file_inode;
    f->f_pos = 0;
    f->f_op = NULL;

    if (file_inode->i_op)
        f->f_op = file_inode->i_op->default_file_ops;

    if (f->f_op && f->f_op->open)
    {
        ret = f->f_op->open(file_inode, f);
        if (ret)
        {
            f->f_count--;
            current->filp[fd] = NULL;
            return ret;
        }
    }
    
    //set f->f_flags 

    return fd;
}

int sys_close(unsigned int fd)
{
	struct file  *file;
    struct inode *file_inode;

	if (fd >= NR_OPEN)
		return -EBADF;
	
    if (!(file = current->filp[fd]))
		return -EBADF;
	
    current->filp[fd] = NULL;

    if (file->f_count == 0) 
    {
        printk("VFS: Close: file count is 0\n");
        return 0;
    }
    file_inode = file->f_inode;
    
    if (file->f_count > 0) 
        file->f_count--;   
    
    if (file->f_op && file->f_op->close)
        file->f_op->close(file_inode, file);

    file->f_inode = NULL;
    
    return 0;
}

int sys_read(unsigned int fd, char *buf, unsigned int count)
{
    struct file  *file;
    struct inode *file_inode;

    if (fd >= NR_OPEN || !(file = current->filp[fd]) || !(file_inode = file->f_inode))
        return -EBADF;

    //check permission
    
    //user_to_kernel

    return file->f_op->read(file_inode, file, buf, count);
}

int sys_write(unsigned int fd, char *buf, unsigned int count)
{
    struct file  *file;
    struct inode *file_inode;

    if (fd >= NR_OPEN || !(file = current->filp[fd]) || !(file_inode = file->f_inode))
        return -EBADF;

    //check permission
    //chedk readonly 
    //user_to_kernel

    return file->f_op->write(file_inode, file, buf, count);
}

int sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    struct file *file;

    if (fd > NR_OPEN || !(file = current->filp[fd]))
        return -EBADF;

    if (file->f_op && file->f_op->ioctl)
        return file->f_op->ioctl(file->f_inode, file, cmd, arg);
    
    return -EINVAL;
}

static int do_mkdir(char * pathname, int mode)
{
	char * basename;
	int namelen, error;
	struct inode *dir;

	error = dir_namei(pathname, &namelen, &basename, NULL, &dir);
	
    if (error)
		return error;

	if (!namelen) 
		return -ENOENT;
	
   // if (IS_RDONLY(dir)) 
   //     return -EROFS;
	
    //if (!permission(dir,MAY_WRITE | MAY_EXEC)) 
	//	return -EACCES;
	
    if (!dir->i_op || !dir->i_op->mkdir) 
   		return -EPERM;
	
	error = dir->i_op->mkdir(dir, basename, namelen, mode);
	
    return error;
}

int sys_mkdir(char * pathname, int mode)
{
    return do_mkdir(pathname, mode);
}


int sys_mknod(char * filename, int mode, unsigned int dev)
{
    int ret;
    char *basename;
    int namelen;
    struct inode *dir;

    if (S_ISDIR(mode))
        return -EPERM;
    
	//mode &= ~current->umask;
    ret = dir_namei(filename, &namelen, &basename, NULL, &dir);
    if (ret)
        return ret;

    if (!namelen)
        return -EROFS;

    //if (IS_RDONLY(dir))
    //return -EACCES;

    //if (!permission())
    //return -EACCES;
    
    if (!dir->i_op || !dir->i_op->mknod)
        return -EPERM;

    return dir->i_op->mknod(dir, basename, namelen, mode, dev);
}

int vfs_init(void)
{
	int ret;

	ret = ramdisk_init();
	if (ret < 0)
	{
		printk("RAMDISK INIT FAILED!!\n");
		panic();
	}

	ret = nand_init();
	if (ret < 0)
	{
		printk("NAND INIT FAILED!!\n");
		panic();
	}

	mount_root();
    
	printk("mkdir /nand: %d\n", sys_mkdir("/nand", 666));
	printk("mkdir /nand/abc %d\n", sys_mkdir("/nand/abc", 666));
    printk("mount 2 /nand/abc %d\n", do_mount(3, "/nand/abc", "ofs", 0));
#if 0
	fd = sys_open("/a", O_CREAT, 0);
    printk("%d\n", sys_mknod("/tty", S_IFCHR, 0));

    for (i = 0; i < 128; i++)
        buf[i] = i;
    printk("%d\n", sys_write(fd, buf, 128));
    sys_close(fd);
    
    memset(buf, 0, 128);

    fd = sys_open("/a", O_CREAT, 0);
    printk("%d\n", sys_read(fd, buf, 64));
    for (i = 0; i < 64; i++)
        printk("%d ", buf[i]);
	printk("\n");
    
    printk("%d\n", sys_read(fd, buf, 64));
    for (i = 0; i < 64; i++)
        printk("%d ", buf[i]);
	sys_close(fd);
#endif
    return 0;
}



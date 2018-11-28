#include "tty.h"
#include "common.h"
#include "error.h"
#include "fs.h"
#include "chr.h"

#define DEFAULT_STD_DEVICE_NAME 	"ttySAC"

struct tty *preferred_tty = NULL;
struct tty *tty_drivers = NULL;

int __call_tty_drivers(char *buf, unsigned int len)
{
	if (!preferred_tty || !preferred_tty->write)
		return -ENODEV;
	
	return preferred_tty->write(preferred_tty, buf, len);
}

int register_tty(struct tty *tty)
{
	struct tty *tmp = tty_drivers;
	
	if (!tty)
		return -ENODEV;

	if (!tty_drivers)
	{
		tty_drivers = tty;
		preferred_tty = tty;
		return 0;
	}

	while (!tmp->next)
		tmp = tmp->next;
	
	tmp->next = tty;

	return 0;
}

//////////////////////////////////////////////////////////////////
int std_out(struct inode *inode, struct file *filp, char *buf, int count)
{
	if (!preferred_tty || !preferred_tty->write)
		return -ENODEV;
	
	return preferred_tty->write(preferred_tty, buf, count);
}

int std_err(struct inode *inode, struct file *filp, char *buf, int count)
{
	if (!preferred_tty || !preferred_tty->write)
		return -ENODEV;
	
	return preferred_tty->write(preferred_tty, buf, count);
}

int std_in(struct inode *inode, struct file *filp, char *buf, int count)
{
	if (!preferred_tty || !preferred_tty->write)
		return -ENODEV;
	
	return preferred_tty->read(preferred_tty, buf, count);
}

struct file_operations stdin_fops;
struct file_operations stdout_fops;
struct file_operations stderr_fops;
#define STDIN_DEV_MAJOR		2
#define STDOUT_DEV_MAJOR	3
#define STDERR_DEV_MAJOR	4

int create_stdin_stdout_stderr_device(void)
{
	int ret;
	 if (!preferred_tty)
		 return -ENODEV;

	 stdin_fops.read = std_in;
	 stdout_fops.write = std_out;
	 stderr_fops.write = std_err;

	ret = register_chrdev(STDIN_DEV_MAJOR, "stdin", &stdin_fops);
	if (ret < 0)
		return ret;
	
	ret = register_chrdev(STDOUT_DEV_MAJOR, "stdout", &stdout_fops);
	if (ret < 0)
		return ret;
	
	ret = register_chrdev(STDERR_DEV_MAJOR, "stderr", &stderr_fops);
	if (ret < 0)
		return ret;
	
	ret = sys_mknod("/dev/stdin", S_IFCHR, STDIN_DEV_MAJOR);
	if (ret < 0)
		return ret;

	ret = sys_mknod("/dev/stdout", S_IFCHR, STDOUT_DEV_MAJOR);
	if (ret < 0)
		return ret;

	return sys_mknod("/dev/stderr", S_IFCHR, STDERR_DEV_MAJOR);
}

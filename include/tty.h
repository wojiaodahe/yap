#ifndef __TTY_H__
#define __TTY_H__

struct tty_driver
{
	char *name;
};

struct tty
{
	char *name;
	int (*write)(struct tty *, const char *, unsigned int);
	int (*read)(struct tty *, const char *, unsigned int);
	unsigned int flags;
	unsigned int index;
	void *data;
	struct tty *next;
	struct tty *prev;
};

extern int register_tty(struct tty *tty);
extern int __call_tty_drivers(char *buf, unsigned int len);

#endif

#include "s3c24xx.h"
#include "fs.h"
#include "syscall.h"
#include "chr.h"
#include "printk.h"

/*
 * leds.c: 循环点亮4个LED
 * 此时MMU已开启，使用虚拟地址
 */


/*
 * LED1,LED2,LED4对应GPB5、GPB6、GPB7、GPB8
 */
#define	 GPB5_out	(1<<(5*2))
#define	 GPB6_out	(1<<(6*2))
#define  GPB7_out	(1<<(7*2))
#define	 GPB8_out	(1<<(8*2))


#define	GPB5_msk	(3<<(5*2))
#define	GPB6_msk	(3<<(6*2))
#define	GPB7_msk	(3<<(7*2))
#define	GPB8_msk	(3<<(8*2))

#if 0
static void led_delay(unsigned long dly)
{
	volatile unsigned int i;
	for (i = 0; i < dly; i++)
		;
}
#endif

void init_led()
{
    GPBCON &= ~(GPB5_msk | GPB6_msk | GPB7_msk | GPB8_msk);
    GPBCON |=  (GPB5_out | GPB6_out | GPB7_out | GPB8_out);
}

int test_led(void)
{
	unsigned long i = 0;

	while(1)
	{
		ssleep(1);
		GPBDAT = (~(i << 5));	 	//根据i的值，点亮LED1,2,3,4
		if (++i == 16)
			i = 0;
	}

//	return 0;
}


int led_open(struct inode *inode, struct file *file)
{
	int ret = 0;

	//printk("led_open\n");
	init_led();
	return ret;
}

void led_close(struct inode *inode, struct file * file)
{
	printk("led_close\n");
}

int led_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	test_led();
	printk("led_ioctl");
	return ret;
}

struct file_operations led_fops;

int led_module_init(void)
{
	int ret;
	int led_major = 6;
	
	led_fops.open  = led_open;
	led_fops.close = led_close;
	led_fops.ioctl = led_ioctl;

	ret = register_chrdev(led_major, "led", &led_fops);
	if (ret < 0)
		return ret;
	return sys_mknod("/led", S_IFCHR, led_major);
}

void led_module_exit(void)
{

}

#include "s3c24xx.h"
#include "s3c24xx.h"
#include "fs.h"
#include "wait.h"
#include "interrupt.h"
#include "printk.h"
#include "kernel.h"
#include "chr.h"

extern int test_led(void);

static char keyflag = 0;
wait_queue_t wq;

/*
 * K1,K2,K3,K4对应GPF1、GPF4、GPF2、GPF0
 */
#define GPF0_int    (0x2 << (0 * 2))
#define GPF1_int    (0x2 << (1 * 2))
#define GPF2_int    (0x2 << (2 * 2))
#define GPF4_int    (0x2 << (4 * 2))

#define GPF0_msk    (3 << (0 * 2))
#define GPF1_msk    (3 << (1 * 2))
#define GPF2_msk    (3 << (2 * 2))
#define GPF4_msk    (3 << (4 * 2))


/*
 * 初始化GPIO引脚为外部中断
 * GPIO引脚用作外部中断时，默认为低电平触发、IRQ方式(不用设置INTMOD)
 */
void init_key_irq(void)
{
    // K1,K2,K3,K4对应的4根引脚设为中断功能
    GPFCON &= ~(GPF0_msk | GPF1_msk | GPF2_msk | GPF4_msk);
    GPFCON |= GPF0_int | GPF1_int | GPF2_int | GPF4_int;

    // 对于EINT4，需要在EINTMASK寄存器中使能它
    EINTMASK &= ~(1 << 4);
    /*
     * 设定优先级：
     * ARB_SEL0 = 00b, ARB_MODE0 = 0: REQ1 > REQ2 > REQ3，即EINT0 > EINT1 > EINT2
     * 仲裁器1、6无需设置
     * 最终：
     * EINT0 > EINT1> EINT2 > EINT4 即K4 > K1 > K3 > K2
     */
    PRIORITY = (PRIORITY & ((~0x01) | ~(0x3 << 7)));

    // EINT0、EINT1、EINT2、EINT4_7使能
    INTMSK &= (~(1 << 0)) & (~(1 << 1)) & (~(1 << 2)) & (~(1 << 4));
}


int key_open(struct inode *inode, struct file *file)
{
	int ret = 0;

	printk("key_open\n");

	return ret;
}

void key_close(struct inode *inode, struct file * file)
{
	printk("led_close\n");
}

int key_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	test_led();
	printk("led_ioctl");
	return ret;
}

int key_read(struct inode *inode, struct file *filp, char *buf, int len)
{

	wait_event(&wq, keyflag == 1);
	keyflag = 0;
	return 1;
}

void key_irq(void *arg)
{
	keyflag = 1;
	wake_up(&wq);
}

struct file_operations key_fops;

int key_module_init(void)
{
	int ret;
	int key_major = 10;

	key_fops.open  = key_open;
	key_fops.close = key_close;
	key_fops.ioctl = key_ioctl;
	key_fops.read  = key_read;
/*
	ret = setup_irq_handler(KEY1_IRQ, key_irq, 0);
	if (ret < 0)
		return ret;
*/
	ret = register_chrdev(key_major, "key", &key_fops);
	if (ret < 0)
		return ret;

	return sys_mknod("/key", S_IFCHR, key_major);
}

void key_module_exit(void)
{
	unregister_chrdev(10);
}

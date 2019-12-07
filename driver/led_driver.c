#include "s3c24xx.h"
#include "platform.h"
#include "fs.h"
#include "error.h"
#include "chr.h"
#include "kmalloc.h"
#include "printk.h"

#define	LED_ON	1
#define	LED_OFF 0

struct led
{
	volatile struct s3c24xx_gpio *gpio_mem;
	unsigned int bit;
};

int platform_led_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct led *led;

	led = (struct led *)get_cdev_private_data(inode->i_dev);
	if (!led)
		return -ENODEV;

	file->private_data = led;
	return ret;
}

void platform_led_close(struct inode *inode, struct file *file)
{
	printk("platform_led_close\n");
}

void led_gpio_init(struct led *led)
{
	led->gpio_mem->gpxcon &= ~(3 << (led->bit * 2));
	led->gpio_mem->gpxcon |= (1 << (led->bit * 2));
}

int platform_led_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct led *led;

	led = get_cdev_private_data(inode->i_dev);
	if (!led)
		return -ENODEV;

	if (cmd == LED_ON)
		led->gpio_mem->gpxdata &= ~(1 << led->bit);
	else if (cmd == LED_OFF)
		led->gpio_mem->gpxdata |= (1 << led->bit);

	//printk("platform_led_ioctl\n");
	return 0;
}

struct file_operations platform_led_fops;

int led_probe(struct platform_device *pdev)
{
    int ret;
    struct resource *res_io;
    struct resource *res_mem;
    struct led *led;
    int led_major = 6;
    char *name;

    static int i = 0;

    res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    printk("io start : %x end: %x\n", res_mem->start, res_mem->end);

	res_io = platform_get_resource(pdev, IORESOURCE_IO, 0);
	printk("bit num: %x\n", res_io->start);

	led = kmalloc(sizeof (struct led));
	if (!led)
		return -ENOMEM;

	name = kmalloc(16);
	if (!name)
	{
		kfree(led);
		return -ENOMEM;
	}

	sprintk(name, "led%d", i);
	ret = register_chrdev(led_major + i, name, &platform_led_fops);
	if (ret < 0)
	{
		kfree(name);
		kfree(led);
		return ret;
	}

	led->bit = res_io->start;
	led->gpio_mem = (volatile struct s3c24xx_gpio *)res_mem->start;
	led_gpio_init(led);
	ret = set_cdev_private_data(led_major + i, led);
	if (ret < 0)
	{
		unregister_chrdev(led_major);
		kfree(name);
		kfree(led);
		return ret;
	}

	ret = sys_mknod(name, S_IFCHR, led_major + i);
	if (ret < 0)
	{
		unregister_chrdev(led_major);
		kfree(name);
		kfree(led);
		return ret;
	}
	i++;
    printk("------------\n");
    return 0;
}

int led_remove(struct platform_device *pdev)
{
    printk("+++++++++++\n");
    return 0;
}
struct platform_driver led_drv;

int led_driver_init()
{
	led_drv.probe        = led_probe;
	led_drv.remove       = led_remove;
	led_drv.id_table     = 0;
	led_drv.driver.name  = "led";

	platform_led_fops.open  = platform_led_open;
	platform_led_fops.ioctl = platform_led_ioctl;

    return platform_driver_register(&led_drv);
}

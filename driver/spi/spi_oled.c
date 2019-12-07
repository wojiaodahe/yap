#include "spi.h"
#include "fs.h"
#include "chr.h"
#include "lib.h"
#include "kmalloc.h"
#include "s3c24xx.h"
#include "error.h"

static int major = 30;
//static struct class *class;

static int spi_oled_dc_pin;
static struct spi_device *spi_oled_dev;
static unsigned char *ker_buf;

static void OLED_Set_DC(char val)
{
    if (val)
        GPGDAT |= (1 << spi_oled_dc_pin);
    else
        GPGDAT &= ~(1 << spi_oled_dc_pin);
}

static void OLEDWriteCmd(unsigned char cmd)
{
    OLED_Set_DC(0); /* command */
    spi_write(spi_oled_dev, &cmd, 1);
    OLED_Set_DC(1); /*  */
}

static void OLEDWriteDat(unsigned char dat)
{
    OLED_Set_DC(1); /* data */
    spi_write(spi_oled_dev, &dat, 1);
    OLED_Set_DC(1); /*  */
}

static void OLEDSetPageAddrMode(void)
{
    OLEDWriteCmd(0x20);
    OLEDWriteCmd(0x02);
}

static void OLEDSetPos(int page, int col)
{
    OLEDWriteCmd(0xB0 + page); /* page address */

    OLEDWriteCmd(col & 0xf);   /* Lower Column Start Address */
    OLEDWriteCmd(0x10 + (col >> 4));   /* Lower Higher Start Address */
}


static void OLEDClear(void)
{
    int page, i;

    for (page = 0; page < 8; page ++)
    {
        OLEDSetPos(page, 0);
        for (i = 0; i < 128; i++)
            OLEDWriteDat(0);
    }
}

void OLEDClearPage(int page)
{
    int i;
    OLEDSetPos(page, 0);
    for (i = 0; i < 128; i++)
        OLEDWriteDat(0);    
}

void OLEDInit(void)
{
    /* 向OLED发命令以初始化 */
    OLEDWriteCmd(0xAE); /*display off*/ 
    OLEDWriteCmd(0x00); /*set lower column address*/ 
    OLEDWriteCmd(0x10); /*set higher column address*/ 
    OLEDWriteCmd(0x40); /*set display start line*/ 
    OLEDWriteCmd(0xB0); /*set page address*/ 
    OLEDWriteCmd(0x81); /*contract control*/ 
    OLEDWriteCmd(0x66); /*128*/ 
    OLEDWriteCmd(0xA1); /*set segment remap*/ 
    OLEDWriteCmd(0xA6); /*normal / reverse*/ 
    OLEDWriteCmd(0xA8); /*multiplex ratio*/ 
    OLEDWriteCmd(0x3F); /*duty = 1/64*/ 
    OLEDWriteCmd(0xC8); /*Com scan direction*/ 
    OLEDWriteCmd(0xD3); /*set display offset*/ 
    OLEDWriteCmd(0x00); 
    OLEDWriteCmd(0xD5); /*set osc division*/ 
    OLEDWriteCmd(0x80); 
    OLEDWriteCmd(0xD9); /*set pre-charge period*/ 
    OLEDWriteCmd(0x1f); 
    OLEDWriteCmd(0xDA); /*set COM pins*/ 
    OLEDWriteCmd(0x12); 
    OLEDWriteCmd(0xdb); /*set vcomh*/ 
    OLEDWriteCmd(0x30); 
    OLEDWriteCmd(0x8d); /*set charge pump enable*/ 
    OLEDWriteCmd(0x14); 

    OLEDSetPageAddrMode();

    OLEDClear();
    
    OLEDWriteCmd(0xAF); /*display ON*/    
}


#define OLED_CMD_INIT       0x100001
#define OLED_CMD_CLEAR_ALL  0x100002
#define OLED_CMD_CLEAR_PAGE 0x100003
#define OLED_CMD_SET_POS    0x100004

int oled_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int page;
    int col;
    
    switch (cmd)
    {
        case OLED_CMD_INIT:
        {
            OLEDInit();
            break;
        }
        case OLED_CMD_CLEAR_ALL:
        {
            OLEDClear();
            break;
        }
        case OLED_CMD_CLEAR_PAGE:
        {
            page = arg;
            OLEDClearPage(page);
            break;
        }
        case OLED_CMD_SET_POS:
        {
            page = arg & 0xff;
            col  = (arg >> 8) & 0xff;
            OLEDSetPos(page, col);
            break;
        }
    }
    return 0;
}

int oled_write(struct inode *inode, struct file *file, char *buf, int count)
{
    if (count > 4096)
        return -EINVAL;

    memcpy(ker_buf, buf, count);
    OLED_Set_DC(1); /* data */
    spi_write(spi_oled_dev, ker_buf, count);
    
    return count;
}


struct file_operations oled_ops = 
{
	.ioctl   = oled_ioctl,
	.write   = oled_write,
};

int spi_oled_probe(struct spi_device *dev)
{

    int ret;
    spi_oled_dev = dev;
    spi_oled_dc_pin = (long)dev->dev.platform_data;

    ker_buf = kmalloc(4096);
    
    /* 注册一个 file_operations */
    ret = register_chrdev(major, "oled", &oled_ops);

    if (ret)
    {
        kfree(ker_buf);
        return ret;
    }

    return sys_mknod("/dev/oled", S_IFCHR, major);
}

int spi_oled_remove(struct spi_device *spi)
{

	unregister_chrdev(major);
    kfree(ker_buf);
    
	return 0;
}


static struct spi_driver spi_oled_drv = {
	.driver = 
    {
		.name	= "oled",
	},
	.probe		= spi_oled_probe,
	.remove		= spi_oled_remove,
};

int spi_oled_init(void)
{
    return spi_register_driver(&spi_oled_drv);
}

void spi_oled_exit(void)
{
    spi_unregister_driver(&spi_oled_drv);
}


#include "io.h"
#include "spi.h"
#include "error.h"
#include "printk.h"
#include "kmalloc.h"
#include "s3c24xx.h"
#include "platform.h"
#include "interrupt.h"
#include "completion.h"
#include "spi_s3c24xx.h"
#include "s3c24xx_irqs.h"

struct s3c24xx_spi_info
{
    unsigned int            bus_num;
    unsigned int            irq;
    unsigned long           reg_base_phy;
    struct spi_master       *master;
    struct completion       done;
    struct platform_device  *pdev;
    unsigned int            cur_cnt;
    struct spi_transfer     *cur_t;
};

int s3c24xx_spi_setup(struct spi_device *dev)
{
    unsigned int spcon = 0;
    unsigned int cpol = 0;
    unsigned int cpha = 0;
    unsigned int div = 0;
    struct s3c24xx_spi_info *info;
	if (!dev)
	{
		printk("dev is NULL\n");
		return -ENODEV;
	}

#if 1
    info = spi_master_get_devdata(dev->master);
    if (!info)
        return -ENODEV;

    if (dev->mode & 0x01)
        cpha = 1;
    if (dev->mode & 0x02)
        cpol = 1;
    
    spcon = (S3C2440_SPCON_SMOD_IRQ | S3C2440_SPCON_ENSCK | S3C2440_SPCON_MSTR | (cpol << 2) | (cpha << 1));
    writeb(spcon, info->reg_base_phy + S3C2440_SPCON);
    
    /*
     *!!!!!!!!!!! 要根据设备的速率计算分频 !!!!!!!!!!!
     */
    div = 2;
    writeb(div, info->reg_base_phy + S3C2440_SPPRE);
#else

    //SPPRE0 = 2;
    SPPRE1 = 2;

    /* [6:5] : 00, polling mode
    * [4]   : 1 = enable 
    * [3]   : 1 = master
    * [2]   : 0
    * [1]   : 0 = format A
    * [0]   : 0 = normal mode
    */
    //SPCON0 = (1 << 5) | (1 << 4) | (1 << 3);
    SPCON1 = (1 << 5) | (1 << 4) | (1 << 3);
#endif

    return 0;
}
    
void s3c24xx_gpio_setpin(unsigned short pin, unsigned int val)
{
    if (val)
        GPFDAT |=  (1 << pin);
    else
        GPFDAT &= ~(1 << pin);
}

unsigned int sent_len = 0;
unsigned int irq_times = 0;
unsigned int current_sent_len = 0;
unsigned int current_snt_remain = 0;
unsigned int spi_irq_times = 0;
unsigned char current_data = 0xff;

int s3c24xx_check_status(struct s3c24xx_spi_info *info)
{
    unsigned long status;

    status = readb(info->reg_base_phy + S3C2440_SPSTA);
    
    status &= 0x07;

    if (status & 0x01)
        return 0;
    else if (!status)
        return -EBUSY;
    else if ((status & (1 << 2)) || (status & (1 << 1)))
        return -EIO;
}


int s3c24xx_spi_transfer(struct spi_device *dev, struct spi_message *msg)
{
    int ret = 0;
    struct list_head *list;
    struct spi_transfer *tr;
    struct s3c24xx_spi_info *info;

    info = spi_master_get_devdata(dev->master);
    if (!info)
        return -ENODEV;

    s3c24xx_gpio_setpin(dev->chip_select, 0);
    
    
    dev->master->setup(dev);

    current_data = 0;

    list = msg->transfers.next;
    while (list != &msg->transfers)
    {
        tr = list_entry(list, struct spi_transfer, list); 
        list = list->next;
        list_del(&tr->list);
        
        sent_len += tr->len;
        current_sent_len = tr->len;
        current_snt_remain = tr->len;
    
        init_completion(&info->done);

        info->cur_t = tr;
        info->cur_cnt = 0;
        
        while (s3c24xx_check_status(info) != 0)
            ;
        
        if (tr->tx_buf)
        {
            writeb(((unsigned char *)tr->tx_buf)[0], info->reg_base_phy + S3C2440_SPDATA);
            current_data = ((unsigned char *)tr->tx_buf)[0];
        }
        else
            writeb(0xff, info->reg_base_phy + S3C2440_SPDATA);

        wait_for_completion(&info->done);
    }
    
    msg->status = 0;
    msg->complete(msg->context);

    s3c24xx_gpio_setpin(dev->chip_select, 1);
    return ret;
}

void s3c24xx_spi_irq(void *data)
{
    struct s3c24xx_spi_info *info;
    struct spi_transfer *t;

    info = data;
    t = info->cur_t;
    if (!t)
    {
        printk("spi t is NULL\n");
        return;
        //panic();
    }

    irq_times++;
    if (t->tx_buf)
    {
        info->cur_cnt++;
        current_snt_remain--;
      
        while (s3c24xx_check_status(info) != 0)
            ;
       
        if (info->cur_cnt < t->len)
        {
            writeb(((unsigned char *)t->tx_buf)[info->cur_cnt], info->reg_base_phy + S3C2440_SPDATA);
            current_data = ((unsigned char *)t->tx_buf)[info->cur_cnt];
            //printk("%x\n", current_data);
        }
        else
            complete(&info->done);
    }
    else
    {
        ((unsigned char *)t->tx_buf)[info->cur_cnt] = readb(info->reg_base_phy + S3C2440_SPDATA);
        if (info->cur_cnt < t->len)
            writeb(0xff, info->reg_base_phy + S3C2440_SPDATA);
        else
            complete(&info->done);

    }
}

void s3c24xx_spi_controler_init(struct s3c24xx_spi_info *info)
{
#if 1
    GPFCON &= ~(3 << (1 * 2));
    GPFCON |=  (1 << (1 * 2));
    GPFDAT |=  (1 << 1);

    GPGCON &= ~((3 << (2 * 2)) | (3 << (4 * 2)) | (3 << (5 * 2)) | (3 << (6 * 2)) | (3 << (7 * 2)));
    GPGCON |=  ((1 << (2 * 2)) | (1 << (4 * 2)) | (3 << (5 * 2)) | (3 << (6 * 2)) | (3 << (7 * 2)));
    GPGDAT |=  (1 << 2);
#else
    /* GPF1 OLED_CSn output */
    GPFCON &= ~(3<<(1*2));
    GPFCON |= (1<<(1*2));
    GPFDAT |= (1<<1);

    /* GPG2 FLASH_CSn output
    * GPG4 OLED_DC   output
    * GPG5 SPIMISO   
    * GPG6 SPIMOSI   
    * GPG7 SPICLK    
    */
    GPGCON &= ~((3<<(2*2)) | (3<<(4*2)) | (3<<(5*2)) | (3<<(6*2)) | (3<<(7*2)));
    GPGCON |= ((1<<(2*2)) | (1<<(4*2)) | (3<<(5*2)) | (3<<(6*2)) | (3<<(7*2)));
    GPGDAT |= (1<<2);
#endif
}

static int create_spi_master_s3c24xx(int bus_num, unsigned long reg_base_phy, int irq)
{
    int ret = 0;
    struct platform_device *pdev;
    struct s3c24xx_spi_info *info;
    struct spi_master *master;

    pdev = platform_alloc_device("s3c24xx-spi", bus_num);
    if (!pdev)
        return -ENOMEM;
    
    master = spi_alloc_master(&pdev->dev, sizeof (struct s3c24xx_spi_info));
    if (!master)
        return -ENODEV;

    info = spi_master_get_devdata(master);

    master->bus_num         = bus_num;
    master->irq             = irq;
    master->num_chipselect  = 0xffff;
    master->setup           = s3c24xx_spi_setup;
    master->transfer        = s3c24xx_spi_transfer;

    info->master            = master;
    info->bus_num           = bus_num;
    info->pdev              = pdev;
    info->irq               = irq;
    info->reg_base_phy      = reg_base_phy;

    s3c24xx_spi_controler_init(info);

    ret = request_irq(irq, s3c24xx_spi_irq, 0, info);
    if (ret < 0)
    {
//      spi_free_master(master); 
//      platform_free_device(pdev);
        printk("request_irq %d error %s\n", irq, __func__);
        panic();
        return ret;
    }

    return spi_register_master(master);
}

void s3c24xx_controler_init(void)
{
    //create_spi_master_s3c24xx(0, 0x59000000, IRQ_SPI0);
    create_spi_master_s3c24xx(1, 0x59000020, IRQ_SPI1);
}





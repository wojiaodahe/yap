#include "common.h"
#include "error.h"
#include "list.h"
#include "bug.h"
#include "spi.h"
#include "device.h"
#include "completion.h"
#include "printk.h"
#include "kmalloc.h"
#include "lib.h"

static struct list_head spi_master_list;
static struct list_head spi_board_list;


static struct spi_driver *to_spi_driver(struct device_driver *drv)
{
    if (!drv)
        return NULL;

    return container_of(drv, struct spi_driver, driver);
}

static struct spi_device *to_spi_device(struct device *dev)
{
    if (!dev) 
    return NULL;

    return container_of(dev, struct spi_device, dev);
}

static int spi_match_id(struct spi_device_id *id, struct spi_device *sdev)
{
	return 0;
}


static int spi_match_device(struct device *dev, struct device_driver *drv)
{
	 struct spi_device	*spi = to_spi_device(dev);
	 struct spi_driver	*sdrv = to_spi_driver(drv);

	if (sdrv->id_table)
		return spi_match_id(sdrv->id_table, spi);

	return (strcmp(spi->name, drv->name) == 0);
}

struct bus_type spi_bus_type = 
{
	.name		= "spi",
	.match		= spi_match_device,
};

void spi_dev_put(struct spi_device *dev)
{

}

static int spi_drv_probe(struct device *dev)
{
    struct spi_driver *spi_drv;

    spi_drv = to_spi_driver(dev->driver);

    return spi_drv->probe(to_spi_device(dev));
}

static int spi_drv_remove(struct device *dev)
{

    struct spi_driver *spi_drv;

    spi_drv = to_spi_driver(dev->driver);

    return spi_drv->remove(to_spi_device(dev));
}

static void spi_drv_shutdown(struct device *dev)
{
    struct spi_driver *spi_drv;

    spi_drv = to_spi_driver(dev->driver);

    spi_drv->shutdown(to_spi_device(dev));
}


int spi_register_driver(struct spi_driver *spi_drv)
{
    spi_drv->driver.bus = &spi_bus_type;

    if (spi_drv->probe)
        spi_drv->driver.probe = spi_drv_probe;

    if (spi_drv->remove)
        spi_drv->driver.remove = spi_drv_remove;

    if (spi_drv->shutdown)
        spi_drv->driver.shutdown = spi_drv_shutdown;

    return driver_register(&spi_drv->driver);
}

void spi_unregister_driver(struct spi_driver *spi_drv)
{
    if (spi_drv)
        driver_unregister(&spi_drv->driver);
}

void spidev_release(struct device *dev)
{
    struct spi_device *spi = to_spi_device(dev); 

    kfree(spi);
}

struct spi_device *spi_alloc_device(struct spi_master *master)
{
    struct spi_device *spi;
//    struct device *dev = master->dev.parent;

//    if (!spi_master_get(master))
//       return NULL;

    spi = kmalloc(sizeof (struct spi_device));
    if (!spi)
    {
        printk("Cannot Alloc spi_device %s\n", __func__);
        return NULL;
    }

    spi->master = master;
    spi->dev.parent = &master->dev;
    spi->dev.bus = &spi_bus_type;
    spi->dev.release = spidev_release;

    device_initialize(&spi->dev);

    return spi;
}

int spi_setup(struct spi_device *dev)
{
    int status = 0;
    
    /*
     * 检查设备的模式和控制器模式是否匹配
     * if (mode ! 匹配)
     *     return -EINVAL;
     */ 

    if (dev->master->setup)
       status = dev->master->setup(dev);
    
    return status;
}

int spi_add_device(struct spi_device *spi)
{
    int status = 0;


    if (spi->chip_select >= spi->master->num_chipselect)
        return -EINVAL;

    //dev_set_name();

    status = spi_setup(spi);
    if (status < 0)
        return status;

    return device_register(&spi->dev);
}

struct spi_device *spi_new_device(struct spi_master *master, struct spi_board_info *chip)
{
    struct spi_device *proxy;
    int status;

    proxy = spi_alloc_device(master);
    if (!proxy)
        return NULL;

    proxy->chip_select          = chip->chip_select;
    proxy->max_speed_hz         = chip->max_speed_hz;
    proxy->mode                 = chip->mode;             
    proxy->irq                  = chip->irq;
    proxy->dev.platform_data    = chip->platform_data;

    status = spi_add_device(proxy);
    if (status < 0)
    {
        spi_dev_put(proxy);
        return NULL;
    }

    return proxy;

}

static void spi_match_master_to_boardinfo(struct spi_master *master, struct spi_board_info *bi)
{
    struct spi_device *dev;

    if (master->bus_num != bi->bus_num)
        return;

    dev = spi_new_device(master, bi);
    if (!dev)
    {

    }
}

static int __spi_async(struct spi_device *spi, struct spi_message *message)
{
	struct spi_master *master = spi->master;

#if 0
	/* Half-duplex links include original MicroWire, and ones with
	 * only one data pin like SPI_3WIRE (switches direction) or where
	 * either MOSI or MISO is missing.  They can also be caused by
	 * software limitations.
	 */
	if ((master->flags & SPI_MASTER_HALF_DUPLEX)
			|| (spi->mode & SPI_3WIRE)) 
    {
		struct spi_transfer *xfer;
		unsigned flags = master->flags;

		list_for_each_entry(xfer, &message->transfers, transfer_list) {
			if (xfer->rx_buf && xfer->tx_buf)
				return -EINVAL;
			if ((flags & SPI_MASTER_NO_TX) && xfer->tx_buf)
				return -EINVAL;
			if ((flags & SPI_MASTER_NO_RX) && xfer->rx_buf)
				return -EINVAL;
		}
	}
#endif

	message->spi = spi;
	message->status = -EINPROGRESS;
	return master->transfer(spi, message);
}

int spi_async(struct spi_device *spi, struct spi_message *message)
{
	int ret = 0;
	struct spi_master *master = spi->master;
//	unsigned long flags;

//	spin_lock_irqsave(&master->bus_lock_spinlock, flags);

	if (master->bus_lock_flag)
		ret = -EBUSY;
	else
		ret = __spi_async(spi, message);

//  spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);

	return ret;
}

int spi_async_locked(struct spi_device *spi, struct spi_message *message)
{
	int ret = 0;
//	struct spi_master *master = spi->master;
//	unsigned long flags;

	//spin_lock_irqsave(&master->bus_lock_spinlock, flags);

	ret = __spi_async(spi, message);

	//spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);

	return ret;

}

static void spi_complete(void *arg)
{
	complete(arg);
}

static int __spi_sync(struct spi_device *spi, struct spi_message *message, int bus_locked)
{
	int status;
	struct completion done;
	struct spi_master *master = spi->master;

    init_completion(&done);

	message->complete = spi_complete;
	message->context = &done;
    message->master = master;

//	if (!bus_locked)
//		mutex_lock(&master->bus_lock_mutex);

	status = spi_async_locked(spi, message);

//	if (!bus_locked)
//		mutex_unlock(&master->bus_lock_mutex);

	if (status == 0) 
    {
		wait_for_completion(&done);
		status = message->status;
	
    }

	message->context = NULL;
	return status;
}

void spi_message_init(struct spi_message *m)
{
    memset(m, 0, sizeof (struct spi_message));
    INIT_LIST_HEAD(&m->transfers);
}

void spi_message_add_tail(struct spi_transfer *t, struct spi_message *m)
{
    list_add_tail(&t->list, &m->transfers);
}

int spi_sync(struct spi_device *spi, struct spi_message *message)
{
	return __spi_sync(spi, message, 0);
}

int spi_write(struct spi_device *dev, void *buf, int len)
{
    struct spi_transfer t = 
    {
        .tx_buf = buf, 
        .len    = len,
    };
    struct spi_message m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);

    return spi_sync(dev, &m);
}

void spi_master_set_devdata(struct spi_master *master, void *data)
{
    dev_set_drvdata(&master->dev, data);
}

void *spi_master_get_devdata(struct spi_master *master)
{
    return dev_get_drvdata(&master->dev);
}

struct spi_master *spi_alloc_master(struct device *dev, unsigned int size)
{
    struct spi_master *master = NULL;

    master = kmalloc(size  + sizeof (*master));
    if (!master)
        return NULL;

    device_initialize(&master->dev);

    spi_master_set_devdata(master, &master[1]);

    return master;
}

int spi_register_master(struct spi_master *master)
{
    int status = 0;
    struct spi_board_info *bi;
    struct list_head *list;

    if (master->num_chipselect == 0)
        return -EINVAL;

    //spin_lock_init(&master->bus_lock_spinlock)
    //mutex_init(&master->bus_lock_mutex);
    
    master->bus_lock_flag = 0;

    status = device_add(&master->dev);
    if (status < 0)
        return status;

    if (master->transfer)
    {

    }
    else
    {
        //status = spi_master_initialize_queue(master);
        if (status < -0)
        {
            device_unregister(&master->dev);
            return status;
        }
    }


    //mutex_lock(&spi_board_lock);
    
    list_add_tail(&master->list, &spi_master_list);
    list_for_each(list, &spi_board_list)
    {
        bi = list_entry(list, struct spi_board_info, list);
        spi_match_master_to_boardinfo(master, bi);
    }
    
    //mutex_unlock(&spi_board_lock);

    return status;
}

int spi_register_board_info(struct spi_board_info *info, unsigned int n)
{
    int i;
    struct list_head *list;
    struct spi_master *master;
    struct spi_board_info *bi;

    bi = kmalloc(n * sizeof (struct spi_board_info));
    if (!bi)
        return -ENOMEM;

    for (i = 0; i < n; i++)
    {
        memcpy(bi, info, sizeof (struct spi_board_info)); 
        //mutex_lock(&spi_board_lock);
       
        list_add_tail(&bi->list, &spi_board_list); 
        list_for_each(list, &spi_master_list)
        {
            master = list_entry(list, struct spi_master, list);
            spi_match_master_to_boardinfo(master, info);
        }
        bi++;
        info++;

        //mutex_unlock(&spi_board_lock);
    }
    
    return 0;
}

void spi_unregister_device(struct spi_device *spi)
{
    if (spi)
        device_unregister(&spi->dev);
}

int __unregister(struct device *dev, void *null)
{
    spi_unregister_device(to_spi_device(dev));
    
    return 0;
}

void spi_unregister_master(struct spi_master *master)
{
    //mutex_lock(&spi_board_lock);
    list_del(&master->list);
    //mutex_unlock(&spi_board_lock);
    
   //device_for_each_child(&master->dev, NULL, __unregister); /* 卸载掉这个控制器下面的所有spi设备 函数实现尚不完整 暂不使用*/
    
    device_unregister(&master->dev);
}

void spi_unregister_board_info(struct spi_board_info *info)
{

}

int spi_module_init(void)
{
    INIT_LIST_HEAD(&spi_master_list);
    INIT_LIST_HEAD(&spi_board_list);
    return bus_register(&spi_bus_type);
}


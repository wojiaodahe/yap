#include "common.h"
#include "device.h"
#include "error.h"
#include "list.h"
#include "printk.h"
#include "lib.h"

static struct list_head bus_list_head;
void bus_list_init(void)
{
    bus_list_head.next = &bus_list_head;
    bus_list_head.prev = &bus_list_head;
}

int bus_register(struct bus_type *bus)
{
    if (!bus)
        return -EINVAL;

    INIT_LIST_HEAD(&bus->dev_head);
    INIT_LIST_HEAD(&bus->drv_head);

    list_add_tail(&bus->list, &bus_list_head);

    return 0;
}

int bus_unregister(struct bus_type *bus)
{
    if (!bus)
        return -EINVAL;

    list_del(&bus->list);

    return 0;
}

struct bus_type *bus_find_bus_head(struct bus_type *bus)
{
    struct list_head *list;
    struct bus_type *tmp;

    list_for_each(list, &bus_list_head)
    {
        tmp = list_entry(list, struct bus_type, list);
        if (strcmp(tmp->name, bus->name) == 0)
            return tmp;
    }

    return NULL;
}

int bus_for_each_dev(struct bus_type *bus, void *data, struct device *dev, int (*fn)(struct device *dev, void *data))
{
    int ret;
    struct list_head *list;
    struct device *tmp;
    struct bus_type *this_bus_head;

    this_bus_head = bus_find_bus_head(bus);
    if (!this_bus_head)
        return 0;

    list_for_each(list, &this_bus_head->dev_head)
    {
        tmp = list_entry(list, struct device, list);
        ret = fn(tmp, data);
        if (ret < 0)
            break;
    }

    return ret;
}

int driver_for_each()
{
    return 0;
}

/*
 * !!!!!!!!!!!!!!!!!!!!BUGS!!!!!!!!!!!!!!!!!!!!!!!!
 * 实现spi框架的过程中添加
 * 功能未实现
 * 未测试
 */
struct device *next_device()
{
    struct device *dev = NULL;
    //struct device_private *dev_pri;

    return dev;
}

/*
 * !!!!!!!!!!!!!!!!!!!!BUGS!!!!!!!!!!!!!!!!!!!!!!!!
 * 实现spi框架的过程中添加
 * 功能未实现
 * 未测试
 */
int device_for_each_child(struct device *parent, void *data, int (*fn)(struct device *dev, void *data))
{

    return 0;
#if 0
    int error = 0;
    struct device *child;
    while (!error)
    {
       child = next_device();
       if (!child)
           break;
       error = fn(child, data);
    }

    return error;
#endif
}

void device_initialize(struct device *dev)
{
    INIT_LIST_HEAD(&dev->list);
    dev->p.device = dev;
}

int driver_register(struct device_driver *drv)
{
	int ret = 0;
    struct device *dev;
    struct list_head *dev_list;
    struct bus_type *this_bus_head;

    if (!drv || !drv->bus || !drv->bus->match)
        return -EINVAL;

    this_bus_head = bus_find_bus_head(drv->bus);
    if (!this_bus_head)
        printk("Unregistered BUS Type: %s\n", drv->bus->name);

    if (!drv->bus->probe && !drv->probe)
        return -EINVAL;

    list_for_each(dev_list, &this_bus_head->dev_head)
    {
        dev = list_entry(dev_list, struct device, list);
        if (drv->bus->match(dev, drv))
        {
            dev->driver = drv;
            if(drv->bus->probe)
               ret = drv->bus->probe(dev);
            else if (drv->probe)
               ret = drv->probe(dev);
			else
				return -EINVAL;
			
			if (ret < 0)
				return ret;
        }
    }

    list_add_tail(&drv->list, &this_bus_head->drv_head);

    return ret;
}

void release_drv_struct(struct device_driver *drv)
{
    if (!drv)
        return;

    memset(drv, 0, sizeof(struct device_driver));
    //free(drv);
}

int driver_unregister(struct device_driver *drv)
{
    struct device *dev;
    struct list_head *dev_list;
    struct bus_type *this_bus_head;

    if (!drv)
        return -EINVAL;

    this_bus_head = bus_find_bus_head(drv->bus);
    if (this_bus_head)
    {
        list_for_each(dev_list, &this_bus_head->dev_head)
        {
            dev = list_entry(dev_list, struct device, list);
            if (drv->bus->match(dev, drv))
            {
                if(drv->bus->remove)
                    drv->bus->remove(dev);
                else if (drv->remove)
                    drv->remove(dev);
                dev->driver = NULL;
            }
        }
    }

    list_del(&drv->list);
    INIT_LIST_HEAD(&drv->list);
    //release_drv_struct(drv);
    return 0;
}

int device_add(struct device *dev)
{
    return 0;
}

void device_del(struct device *dev)
{

}

int dev_set_drvdata(struct device *dev, void *data)
{
    dev->p.driver_data  = data;
    
    return 0;
}

void *dev_get_drvdata(struct device *dev)
{
    if (dev)
        return dev->p.driver_data;

    return NULL;
}

int device_register(struct device *dev)
{
	int ret = 0;
    struct device_driver *drv;
    struct list_head *drv_list;
    struct bus_type *this_bus_head;

    if (!dev || !dev->bus || !dev->bus->match)
        return -EINVAL;

    this_bus_head = bus_find_bus_head(dev->bus);
    if (!this_bus_head)
    {
        printk("Unregistered BUS Type: %s %s\n", dev->bus->name, __func__);
		return -EINVAL;
    }
	
    list_for_each(drv_list, &this_bus_head->drv_head)
    {
        drv = list_entry(drv_list, struct device_driver, list);
        if (drv->bus->match(dev, drv))
        {
            dev->driver = drv;
            if(drv->bus->probe)
                ret = drv->bus->probe(dev);
            else if (drv->probe)
                ret = drv->probe(dev);
            break;
        }
    }
			
    if (ret < 0)
	    dev->driver = NULL;
    else
        list_add_tail(&dev->list, &this_bus_head->dev_head);

    return ret;
}

void release_dev_struct(struct device *dev)
{
    if (!dev)
        return;

    memset(dev, 0, sizeof(struct device));
    //free(dev);
}

int device_unregister(struct device *dev)
{
    if (!dev)
        return -EINVAL;

    if (dev->bus && dev->bus->remove)
        dev->bus->remove(dev);
    else if (dev->driver && dev->driver->remove)
        dev->driver->remove(dev);

    dev->driver = NULL;
    list_del(&dev->list);
    INIT_LIST_HEAD(&dev->list);
    //release_dev_struct(dev);
    //free(dev)
    return 0;
}

#if 0

void print_bus()
{
    struct list_head *list;
    struct bus_type *tmp;

    list_for_each(list, &bus_list_head)
    {
        tmp = list_entry(list, struct bus_type, list);
        printk("%s\n", tmp->name);
    }

}

void print_devices()
{
    struct list_head *dev_list;
    struct list_head *bus_head;
    struct bus_type *bus_tmp;
    struct device *dev_tmp;

    list_for_each(bus_head, &bus_list_head)
    {
        bus_tmp = list_entry(bus_head, struct bus_type, list);
        printk("Bus Type: %s\n", bus_tmp->name);
        list_for_each(dev_list, &bus_tmp->dev_head)
        {
            dev_tmp = list_entry(dev_list, struct device, list);
            printk("%s\n", dev_tmp->name);
        }
    }
}

void print_drivers()
{
    struct list_head *drv_list;
    struct list_head *bus_head;
    struct bus_type *bus_tmp;
    struct device_driver *drv_tmp;

    list_for_each(bus_head, &bus_list_head)
    {
        bus_tmp = list_entry(bus_head, struct bus_type, list);
        printk("Bus Type: %s\n", bus_tmp->name);
        list_for_each(drv_list, &bus_tmp->drv_head)
        {
            drv_tmp = list_entry(drv_list, struct device_driver, list);
            printk("%s\n", drv_tmp->name);
        }
    }
}

#define BUS_TYPE_NUM    5
#define DEVICE_NUM      10

int probe(struct device *dev)
{
    printk("probe: dev: %s\n", dev->name);
    return 0;
}
int match(struct device *dev, struct device_driver *drv)
{

    printk("match: dev: %s drv: %s\n", dev->name, drv->name);

    return 1;
}

struct bus_type bus[BUS_TYPE_NUM];
struct device_driver drv[BUS_TYPE_NUM];

struct dev_arr
{
    struct device dev[DEVICE_NUM];
};

struct dev_arr dev_arr[BUS_TYPE_NUM];

int test_device(int argc, char *argv[])
{
    int k, i;

    bus_list_init();

    for (i = 0; i < BUS_TYPE_NUM; i++)
    {
        sprintk(bus[i].name, "bus%d", i);
        bus[i].match = match;
        bus_register(&bus[i]);
    }
    print_bus();

#if 1
    for (i = 0; i < BUS_TYPE_NUM; i++)
    {
        sprintk(drv[i].name, "bus%ddriver", i);
        drv[i].bus = &bus[i];
        drv[i].probe = probe;
        driver_register(&drv[i]);
    }
#endif
    for (k = 0; k < BUS_TYPE_NUM; k++)
    {
        for (i = 0; i < DEVICE_NUM; i++)
        {
            sprintk(dev_arr[k].dev[i].name, "bus%ddev%d", k, i);
            dev_arr[k].dev[i].bus = &bus[k];
            device_register(&dev_arr[k].dev[i]);
        }
    }

    print_devices();

    driver_unregister(&drv[1]);
    driver_unregister(&drv[3]);
    print_drivers();
    for (i = 0; i < BUS_TYPE_NUM; i++)
        printk("drv[%d].name: %s\n", i, drv[i].name);


    for (k = 0; k < BUS_TYPE_NUM; k++)
    {
        device_unregister(&dev_arr[k].dev[1]);
        device_unregister(&dev_arr[k].dev[3]);
        device_unregister(&dev_arr[k].dev[6]);
        device_unregister(&dev_arr[k].dev[9]);
    }
    print_devices();
    return 0;
}

#endif

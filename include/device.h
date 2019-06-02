#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "list.h"

typedef int     dev_t;

struct bus_type_private
{
    struct bus_type *bus;
};

struct driver_private
{
    struct device_driver *driver;
};

struct device_private
{
    void *driver_data;
    struct device *device;
};


struct device
{
    struct bus_type *bus;
    struct device_driver *driver;
    struct list_head list;

    struct device_private p;
    void *platform_data;
    struct device *parent;

    void (*release)(struct device *dev);
};

struct device_driver
{
    char *name;
    struct bus_type *bus;
    int (*match)(struct device * dev, struct device_driver *drv);
    int (*probe)(struct device *dev);
    int (*remove)(struct device *dev);
    void (*shutdown)(struct device *dev);
    struct driver_private *p;
    struct list_head list;
};

struct bus_type
{
    char *name;
    int (*match)(struct device *dev, struct device_driver *drv);
    int (*probe)(struct device *dev);
    int (*remove)(struct device *dev);
    struct bus_type_private *p;

    struct list_head list;
    struct list_head dev_head;
    struct list_head drv_head;
};


#define MAX_CHRDEV 32
#define MAX_BLKDEV 32

extern struct device *alloc_device_struct(void);
int put_device_struct(struct device *dev);

extern int device_unregister(struct device *);
extern int device_register(struct device *);
extern int driver_register(struct device_driver *);
extern int driver_unregister(struct device_driver *);
extern int bus_register(struct bus_type *);
extern void device_initialize(struct device *);
extern void bus_list_init(void);
extern int bus_register(struct bus_type *bus);
extern int device_add(struct device *dev);
extern void device_del(struct device *dev);
extern int device_for_each_child(struct device *parent, void *data, int (*fn)(struct device *dev, void *data));
extern int dev_set_drvdata(struct device *dev, void *data);
extern void * dev_get_drvdata(struct device *dev);

#endif 


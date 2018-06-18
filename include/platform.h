#ifndef __PLATFORM_H__
#define __PLATFORM_H__
#include "device.h"

#define PLATFORM_NAME_SIZE  20

typedef unsigned long resource_size_t;

struct resource
{
    resource_size_t start;
    resource_size_t end;
    const char *name;
    unsigned long flag;
};

#define IORESOURCE_IO       0x00000100
#define IORESOURCE_MEM      0x00000200
#define IORESOURCE_IRQ      0x00000400
#define IORESOURCE_DMA      0x00000800

struct platform_device_id
{
    char name[PLATFORM_NAME_SIZE];
};

struct platform_device
{
    const char *name;
    int id;
    struct device dev;
    unsigned int num_resource;
    struct resource *resource;
    struct platform_device_id *id_entry;
    void *device_data;
};

struct platform_driver
{
    int (*probe)(struct platform_device *);
    int (*remove)(struct platform_device *);
    struct device_driver driver;
    struct platform_device_id *id_table;
};

extern struct resource *platform_get_resource(struct platform_device *pdev, unsigned int type, unsigned int num);
#endif



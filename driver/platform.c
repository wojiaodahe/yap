#include "device.h"
#include "error.h"
#include "list.h"
#include "platform.h"
#include "common.h"
#include "lib.h"
#include "kmalloc.h"

#define to_platform_driver(drv) container_of((drv), struct platform_driver, driver)
#define to_platform_device(dev) container_of((dev), struct platform_device, dev)

static struct platform_device_id *platform_match_id(struct platform_device_id *id, struct platform_device *pdev)
{
    return NULL;
}

static int platform_match(struct device *dev, struct device_driver *drv)
{
    struct platform_device *pdev;
    struct platform_driver *pdrv;

    pdev = to_platform_device(dev);
    pdrv = to_platform_driver(drv);

    if (pdrv->id_table)
        return platform_match_id(pdrv->id_table, pdev) != NULL;

    return (strcmp(pdev->name, drv->name) == 0);
}

static struct bus_type platform_bus_type;
int platform_bus_init(void)
{

	platform_bus_type.name = "platform";
	platform_bus_type.match = platform_match;

    return bus_register(&platform_bus_type);
}

static int platform_drv_probe(struct device *dev)
{
    struct platform_driver *pdrv;
    struct platform_device *pdev;

    pdev = to_platform_device(dev);
    pdrv = to_platform_driver(dev->driver);

    if (!pdrv->probe)
        return -EINVAL;

    return pdrv->probe(pdev);
}

static int platform_drv_remove(struct device *dev)
{
    struct platform_driver *pdrv;
    struct platform_device *pdev;

    pdev = to_platform_device(dev);
    pdrv = to_platform_driver(dev->driver);

    if (!pdrv->remove)
        return -EINVAL;

    return pdrv->remove(pdev);
}

struct resource *platform_get_resource(struct platform_device *pdev, unsigned int type, unsigned int num)
{
    int i;
    struct resource *res;

    for (i = 0; i < pdev->num_resource; i++)
    {
        res = &pdev->resource[i];
        if (type == res->flag && num-- == 0)
            return res;
    }

    return NULL;
}

struct platform_device *platform_alloc_device(char *name, int id)
{
    struct platform_device *pdev = NULL;

    pdev = kmalloc(sizeof (struct platform_device));
    if (!pdev)
        return NULL;

    pdev->id    = id;
    pdev->name  = name;

    device_initialize(&pdev->dev);

    return pdev;
}

int platform_driver_register(struct platform_driver *pdrv)
{
    pdrv->driver.bus = &platform_bus_type;
    pdrv->driver.probe = platform_drv_probe;
    pdrv->driver.remove = platform_drv_remove;

    return driver_register(&pdrv->driver);
}

void platform_driver_unregister(struct platform_driver *pdrv)
{
    driver_unregister(&pdrv->driver);
}

static int platform_device_add(struct platform_device *pdev)
{
   if (!pdev)
       return -EINVAL;

   pdev->dev.bus = &platform_bus_type;

   return device_register(&pdev->dev);
}

int platform_device_register(struct platform_device *pdev)
{
    device_initialize(&pdev->dev);
    return platform_device_add(pdev);
}

int platform_device_unregister(struct platform_device *pdev)
{
   return device_unregister(&pdev->dev);
}

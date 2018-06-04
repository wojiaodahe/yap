#include "common.h"
#include "device.h"


static struct device devices[MAX_CHRDEV + MAX_BLKDEV];

struct device *alloc_device_struct()
{
    int dev = 0;

    for (dev = 0; dev < MAX_CHRDEV + MAX_BLKDEV; dev++)
    {
        if (devices[dev].dev == 0)
            return &devices[dev];
    }
    
    return NULL;
}

int put_device_struct(struct device *dev)
{
    if (dev->next)
        dev->next->prev = dev->prev;
    if (dev->prev)
        dev->prev->next = dev->next;

    memset(dev, 0, sizeof (struct device));

    return 0;
}

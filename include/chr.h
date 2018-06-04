#ifndef __CHAR_DEV_H__
#define __CHAR_DEV_H__

struct chr_dev_struct 
{
    struct device *device;
};

#define NR_CHRDEV   MAX_CHRDEV

#endif



#include "s3c24xx.h"
#include "machine.h"
#include "mach-s3c24xx.h"
#include "platform.h"

int s3c24xx_set_gpio_mode(unsigned int group, unsigned int bit, unsigned mode)
{

}

int s3c24xx_set_gpio(unsigned int group, unsigned int bit)
{

}

unsigned int s3c24xx_get_gpio(unsigned group, unsigned bit)
{
    return 0;
}

void s3c24xx_enable_irq(unsigned int irq, unsigned int flags)
{

}

void s3c24xx_disable_irq(unsigned int irq)
{

}

int s3c24xx_request_irq(unsigned int irq, unsigned int mode)
{

}

unsigned int s3c24xx_get_irq(void)
{
    return 0;
}

int s3c24xx_mmap(unsigned long paddr, unsigned long vaddr, unsigned int size)
{
    return 0;
}

void s3c24xx_sys_timer_init(void)
{

}

void s3c24xx_sys_timer_suspend(void)
{

}

void s3c24xx_sys_timer_resume(void)
{

}

struct resource dm9000_resurce;


static struct sys_timer s3c24xx_sys_timer = 
{
    .init       = s3c24xx_sys_timer_init,
    .suspend    = s3c24xx_sys_timer_suspend,
    .resume     = s3c24xx_sys_timer_resume,
};

void s3c24xx_machine_init(void)
{

}

static struct machine_desc s3c24xx_desc = 
{
    .nr             = S3C24xx_MACHINE_ID,
    .name           = "mach-s3c24xx",
    .machine_init   = s3c24xx_machine_init,
    .sys_timer      = &s3c24xx_sys_timer,
};

void mach_s3c24xx_entry(void)
{
    setup_machine_desc(&s3c24xx_desc);
}



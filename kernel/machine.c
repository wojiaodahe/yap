#include "arch.h"
#include "machine.h"
#include "common.h"

static struct machine_desc *MACHINE_DESC = NULL;

void setup_machine_desc(struct machine_desc *md)
{
    if (!MACHINE_DESC && md)
        MACHINE_DESC = md;
}

void sys_timer_init(void)
{
    s3c24xx_timer_init();
}

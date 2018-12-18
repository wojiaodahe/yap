#include "machine.h"
#include "common.h"
#include "interrupt.h"

extern struct machine_desc *MACHINE_DESC;

void sys_timer_init(void)
{
    timer_init();
}

void kernel_previous_init(void)
{
    int ret;
    struct machine_desc *md;

    md = MACHINE_DESC;
	
    ret = system_mm_init();
	if (ret < 0)
		panic();
   
    if (!md->machine_init || !md->irq_init || !md->tty_init)
        panic();
    
    md->irq_init();
    md->machine_init();
    md->tty_init();
}

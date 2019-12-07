#include "interrupt.h"
#include "printk.h"

void assertion_failure(char *exp, char *file, char *base_file, int line)
{

	printk("assert(%s) failed: file: %s, base_file: %s, line: %d\r\n",
			exp, file, base_file, line);

	/**
	 * If assertion fails in a TASK, the system will halt before
	 * printl() returns. If it happens in a USER PROC, printl() will
	 * return like a common routine and arrive here. 
	 * @see sys_printx()
	 * 
	 * We use a forever loop to prevent the proc from going on:
	 */
	//spin("assertion_failure()");

}

void check_addr(void *addr)
{
    if (!addr)
    {
        kernel_disable_irq();
        while (1)
            printk("Addr NULL! %s %d\n", __func__, __LINE__);
    }
    
    if (!((unsigned int)addr & 0x30000000))
    {
        kernel_disable_irq();
        while (1)
            printk("Addr: %p Error! %s %d\n", addr, __func__, __LINE__);
    }

    if ((unsigned int)addr & 0xc0000000)
    {
        kernel_disable_irq();
        while (1)
            printk("Addr %p Error! %s %d\n", addr, __func__, __LINE__);
    }
    
}

#include "s3c24xx.h"
#include "kernel.h"
#include "interrupt.h"


void enable_irq(void)
{
	int r4;
	__asm  
	{
		mrs r4, cpsr
		bic r4, r4, #0x80
		msr cpsr_cxsf, r4
	}
}

void disable_irq(void)
{
	int r4;
	__asm 
	{
		mrs r4, cpsr
		orr r4, r4, #0x80
		msr cpsr_cxsf, r4
	}
}

void umask_int(unsigned int offset)
{
	INTMSK &= ~(1 << offset);
}

void usubmask_int(unsigned int offset)
{
    INTSUBMSK &= ~(1 << offset);
}


static irq_handler irq_table[MAX_IRQ_NUMBER];
int put_irq_handler(unsigned int irq_num, irq_server irq_handler, void *priv)
{
    int i;

    for (i = 0; i < MAX_IRQ_NUMBER; i++)
    {
        if (irq_table[i].irq_num == 0)
        {
            //disable_irq();
            irq_table[i].irq_num = irq_num;
            irq_table[i].irq_handler = irq_handler;
            irq_table[i].priv = priv;
            //enable_irq();
            return 0;
        }
    }

    return -1;
}

void do_irq(int irq_num)
{
    int i;

    for (i = 0; i < MAX_IRQ_NUMBER; i++)
    {
        if (irq_table[i].irq_num == irq_num)
        {
            irq_table[i].irq_handler(irq_table[i].priv);
            break;
        }
    }
}

void common_irq_handler()
{
	int bit;
    unsigned long oft = INTOFFSET;

    SRCPND |= (1 << oft);
    INTPND |= (1 << oft);

	//~{GeVP6O~}
    if (oft == 4) 
    {
        for (bit = 4; bit < 24; bit++)
        {
        	if (EINTPEND & (1 << bit))
        	{
        		oft = bit;
        		break;
        	}
        }
        EINTPEND |= (1 << bit);   // EINT4_7~{:OSC~}IRQ4
    }

#if 0
    switch (oft)
    {
        // K1~{1;04OB~}
        case 1: 
        {   
            GPBDAT |= (0xF << 5);     // ~{KySP~}LED~{O(Cp~}
            GPBDAT &= ~~(1 << 5);      // LED1~{5cAA~}
            break;
        }
        
        // K2~{1;04OB~}
        case 4:
        {   
            GPBDAT |= (0xF << 5);   // ~{KySP~}LED~{O(Cp~}
            GPBDAT &= ~~(1 << 6);      // LED2~{5cAA~}
			printk("---------------\r\n");
            break;
        }

        // K3~{1;04OB~}
        case 2:
        {   
            GPBDAT |= (0xF << 5);   // ~{KySP~}LED~{O(Cp~}
            GPBDAT &= ~~(1 << 7);      // LED3~{5cAA~}
			printk("---------------\r\n");
            break;
        }

        // K4~{1;04OB~}
        case 0:
        {   
            GPBDAT |= (0xF << 5);   // ~{KySP~}LED~{O(Cp~}
            GPBDAT &= ~~(1 << 8);      // LED4~{5cAA~}
			printk("---------------\r\n");
            break;
        }

        default:
            break;
    }
	//enable_irq();
	#endif

    do_irq(oft);
}


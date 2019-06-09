#include "s3c24xx.h"
#include "interrupt.h"
#include "s3c24xx_irqs.h"
#include "printk.h"
#include "kmalloc.h"
#include "s3c24xx_irqs.h"

inline void enable_irq(void)
{
	int r4;
	__asm  
	{
		mrs r4, cpsr
		bic r4, r4, #0x80
		msr cpsr_cxsf, r4
	}
}

inline void disable_irq(void)
{
	int r4;
	__asm 
	{
		mrs r4, cpsr
		orr r4, r4, #0x80
		msr cpsr_cxsf, r4
	}
}

inline unsigned long arch_local_irq_save(void)
{
    unsigned flags, r4;

    __asm
    {
		mrs r4, cpsr
        mov flags, r4
		orr r4, r4, #0x80
		msr cpsr_cxsf, r4
    }

    return flags;
}

inline unsigned long arch_local_save_flags(void)
{
	int cp;
	
    __asm 
	{
		mrs cp, cpsr
	}

    return cp;
}

inline void arch_local_restore(unsigned int flags)
{
	__asm 
	{
		msr cpsr_cxsf, flags
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

extern unsigned int spi_irq_times;
unsigned long SRCPND_SPI_IRQ = 0;
unsigned long INTPND_SPI_IRQ = 0;
unsigned long SRC_TIMER_AND_SPI  = 0;
unsigned long INT_TIMER_AND_SPI  = 0;
void common_irq_handler()
{
	int bit;
    int flag = 0;
    unsigned long oft = INTOFFSET;
    unsigned long SRC;
    unsigned long INT;

    SRC = SRCPND;

    if (SRC & (1 << 29))
        SRCPND_SPI_IRQ++;

    if ((SRC & (1 << 29)) && (SRC != (1 << 29)))
    {
        SRC_TIMER_AND_SPI++;
        printk("SRCPND: %x oft: %d\n", SRC, oft);
        flag = 1;
    }

    SRCPND = (1 << oft);
    if (flag)
    {
        printk("After Write SRCPND: %x\n", SRCPND);
        flag = 0;
    }

    if (INT & (1 << 29))
        INTPND_SPI_IRQ++;

    if ((INT & (1 << 29)) && (INT != (1 << 29)))
        INT_TIMER_AND_SPI++;

    INTPND |= (1 << oft);

    if (oft == 29)
        spi_irq_times++;

	//~{GeVP6O~}
    if (oft == IRQ_EINT4t7 || oft == IRQ_EINT8t23) 
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

        oft += EXTINT_OFFSET;
    }

    deliver_irq(oft);
}

void s3c24xx_irq_mask(unsigned int irq_num)
{

    if (irq_num <= IRQ_ADC)
	    INTMSK |= (1 << irq_num);
}

void s3c24xx_irq_unmask(unsigned int irq_num)
{
    if (irq_num <= IRQ_ADC)
	    INTMSK &= ~(1 << irq_num);
}

void s3c24xx_irq_set_flag(unsigned irq_num, unsigned int flag)
{

}

void s3c24xx_irqext_mask(unsigned int irq_num)
{
    if (irq_num <= IRQ_EINT3)
        INTMSK |= (1 << irq_num);
    else if ((irq_num >= IRQ_EINT4) && (irq_num <= IRQ_EINT23))
        EINTMASK |= (1 << (irq_num - EXTINT_OFFSET));
}

void s3c24xx_irqext_unmask(unsigned int irq_num)
{
    if (irq_num <= IRQ_EINT3) 
        INTMSK &= ~(1 << irq_num);
    else if ((irq_num >= IRQ_EINT4) && (irq_num <= IRQ_EINT7))
    {   
        INTMSK   &= ~(1 << 4);
        EINTMASK &= ~(1 << (irq_num - EXTINT_OFFSET));
    }
    else if ((irq_num >= IRQ_EINT4) && (irq_num <= IRQ_EINT7))
    {   
        INTMSK   &= ~(1 << 5);
        EINTMASK &= ~(1 << (irq_num - EXTINT_OFFSET));
    }
    else
    {

    }
}

void s3c24xx_irqext_set_flag(unsigned irq_num, unsigned int flag)
{
    if (irq_num < IRQ_EINT4)
        s3c24xx_irq_set_flag(irq_num, flag);

    if ((irq_num >= IRQ_EINT4) && (irq_num <= IRQ_EINT23))
    {
        switch (flag)
        {
        case IRQ_FLAG_LOW_LEVEL:
            break;
        case IRQ_FLAG_HIGH_LEVEL:
            break;
        case IRQ_FLAG_FALLING_EDGE:
            break;
        case IRQ_FLAG_RISING_EDGE:
            break;
        case IRQ_FLAG_BOTH_EDGE:
            break;
        default:
            break;
        }
    }
}

void s3c24xx_init_irq(void)
{
    unsigned int irq_num;
    struct irq_desc *desc;

    for (irq_num = IRQ_EINT0; irq_num <= IRQ_EINT23; irq_num++)
    {
        desc = kmalloc(sizeof (struct irq_desc));
        if (!desc)
        {
            printk("%s failed!\n", __func__);
            panic();
        }
        desc->irq_num   = irq_num;
        if ((irq_num <= IRQ_EINT3) || (irq_num >= IRQ_EINT4 && irq_num <= IRQ_EINT23))
        {
            desc->mask      = s3c24xx_irqext_mask;
            desc->unmask    = s3c24xx_irqext_unmask;
            desc->set_flag  = s3c24xx_irqext_set_flag;
        }
        /*
         * else if (irq has sub_irq)
         * {
         * xxxxxxxxxxxxxxxxxxxxxxxxxxxx
         * }
         * */
        else
        {
            desc->mask      = s3c24xx_irq_mask;
            desc->unmask    = s3c24xx_irq_unmask;
            desc->set_flag  = s3c24xx_irq_set_flag;
        }

        if (register_irq_desc(desc) < 0)
        {
            printk("register_irq_desc failed! \n");
            panic();
        }

    }
}



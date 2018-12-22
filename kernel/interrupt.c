#include "kernel.h"
#include "error.h"
#include "interrupt.h"
#include "common.h"
#include "kmalloc.h"

static struct irq_desc *irq_desc_table[MAX_IRQ_NUMBER];

void deliver_irq(int irq_num)
{
    struct irq_desc *desc;

    if (irq_num >= MAX_IRQ_NUMBER)
        return;

    desc = irq_desc_table[irq_num];
    
    if (desc && desc->irq_handler)
    {
        desc->count++;
        desc->irq_handler(desc->priv);
    }
}

int register_irq_desc(struct irq_desc *desc)
{
    int irq_num;

    if (!desc)
        return -EINVAL;

    for (irq_num = 0; irq_num < MAX_IRQ_NUMBER; irq_num++)
    {
        if (irq_desc_table[irq_num] == NULL)
        {
            irq_desc_table[irq_num] = desc;
            return 0;
        }
    }

    return -ENOMEM;
}

void unregister_irq_desc(unsigned int irq_num)
{
    struct irq_desc *desc;
    
    if (irq_num >= MAX_IRQ_NUMBER)
        return;
    desc = irq_desc_table[irq_num];

    kfree(desc);
    irq_desc_table[desc->irq_num] = NULL;
}

int request_irq(int irq_num, irq_server irq_handler, unsigned int flag, void *priv)
{
    struct irq_desc *desc;
    
    if (!irq_handler)
        return -EINVAL;

    if (irq_num >= MAX_IRQ_NUMBER)
        return -ENOMEM;
   
    desc = irq_desc_table[irq_num];
    if (desc->irq_handler) 
        return -EBUSY;
    
    desc->priv = priv;
    desc->irq_handler = irq_handler;
    desc->set_flag(irq_num, flag);
    desc->unmask(irq_num);

    return 0;
}

void free_irq(int irq_num)
{
    struct irq_desc *desc;

    if (irq_num >= MAX_IRQ_NUMBER)
        return;
    
    desc = irq_desc_table[irq_num];
    if (!desc)
        return;

    
    desc->mask(irq_num);
    
    desc->irq_handler = NULL;
    desc->flag = 0;
    desc->count = 0;

    /*
     * wakt_up(all wait for threads)
     * */
}



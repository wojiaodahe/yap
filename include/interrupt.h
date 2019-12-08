#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "arch.h"
#include "wait.h"

#define MAX_IRQ_NUMBER      64

typedef void (* irq_server)(void *);

typedef struct
{
    irq_server	 irq_handler;
    unsigned int irq_num;
    void 		 *priv;
}irq_handler;

enum irq_flag 
{
    IRQ_FLAG_LOW_LEVEL = 1,
    IRQ_FLAG_HIGH_LEVEL,
    IRQ_FLAG_FALLING_EDGE,
    IRQ_FLAG_RISING_EDGE,
    IRQ_FLAG_BOTH_EDGE,
};

/* 简易版的中断描述符 */
struct irq_desc
{
    unsigned int irq_num;
    char *name;

    void *priv;
    void (*irq_handler)(void *priv);
    
    void (*set_flag)(unsigned int irq_num, unsigned int flags);
    void (*mask)(unsigned int irq_num);
    void (*unmask)(unsigned int irq_num);

    unsigned int flag;     /* 低电平 高电平 下降沿 上升沿 等*/
    unsigned int count;     /* 此中断发生的次数  */

    wait_queue_t wait_for_threads; /* 等待此中断的线程 */

#ifdef CONFIG_SMP
    unsigned int cpu; /* 中断提交给哪个cpu  */
#endif
};

#define local_irq_disable() kernel_disable_irq()
#define local_irq_enable()  kernel_enable_irq()

#define raw_local_irq_save(flags)\
    do {\
        flags = arch_local_irq_save();\
    } while (0);

#define raw_local_irq_restore(flags)\
    do {\
        arch_local_irq_restore(flags);\
    } while (0);

#define local_irq_save(flags)\
    do {\
        raw_local_irq_save(flags);\
    } while (0);

#define local_irq_restore(flags) \
    do {\
        raw_local_irq_restore(flags);\
    } while (0);

extern int setup_irq_handler(unsigned int , irq_server, void *);
extern void kernel_enable_irq(void);
extern void kernel_disable_irq(void);
extern int request_irq(int irq_num, irq_server irq_handler, unsigned int flag, void *priv);
extern void enter_critical(void);
extern void exit_critical(void);
extern void deliver_irq(int irq_num);
extern int register_irq_desc(struct irq_desc *desc);

#endif


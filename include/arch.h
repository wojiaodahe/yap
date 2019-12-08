#ifndef __ARCH_H__
#define __ARCH_H__

extern void enable_irq(void);
extern void disable_irq(void);
extern void s3c24xx_init_irq(void);
extern unsigned long arch_local_irq_save(void);
extern void arch_local_irq_restore(unsigned int flags);
extern unsigned long arch_local_save_flags(void);
extern void umask_int(unsigned int offset);
extern void usubmask_int(unsigned int offset);
extern int s3c24xx_init_tty(void);
extern int s3c24xx_timer_init(void);
extern void s3c24xx_timer4_irq_handler(void *prv);;


#endif


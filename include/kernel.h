#ifndef __KERNEL_H__
#define __KERNEL_H__

#define HZ                      100

#define OS_INIT_PROCESS_PID		0
#define OS_SYS_PROCESS_PID		1
#define OS_IDLE_PROCESS_PID		2
#define OS_NETIF_PROCESS_PID    3 

extern int sys_init_sys_timer(void);
extern int sys_init_tty(void);
extern int sys_init_irq(void);

#endif


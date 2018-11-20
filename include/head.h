#ifndef __HEAD__H__
#define __HEAD__H__
#include "message.h"
#include "pcb.h"

extern void OS_Sched(void);
extern void printk(char *fmt,...);
extern int test_led(void);
extern void init_led(void);
extern void init_uart0(void);
extern void init_key_irq(void);
extern void put_char(unsigned char);
extern int OS_Init(void);
extern int create_process(int (*f)(void *), void *args, int);
extern void OS_Start(void);
extern void umask_int(unsigned int offset);
extern void usubmask_int(unsigned int offset);
extern void enable_irq(void);
extern void disable_irq(void);
extern int send_recv(int function, int src_dest, MESSAGE* msg);
extern void block(pcb_t* p);
extern void unblock(pcb_t *p);
extern int proc2pid(pcb_t *proc);
extern pcb_t *pid2proc(int pid);
extern int sys_sendrec(int function, int src_dest, MESSAGE* m, pcb_t* p);
extern int msg_sendrec(int type, int src_dest, MESSAGE *msg);
unsigned int OS_Get_Ticks(void);
extern void reset_msg(MESSAGE* p);
extern int syscall(int, unsigned long *);
extern int do_fork(int (*f)(void *), void *args, int pid);
extern int sendrec(int type, int src_dest, MESSAGE *msg);
extern int system_mm_init(void);
extern int vfs_init(void);
extern int create_pthread(int (*f)(void *), void *args, int pid);
extern void memset(void *src, unsigned int num, unsigned int len);
extern void *memcpy(void *dest, void *src, unsigned int len);
extern int strncmp(char *s1, char *s2, unsigned int n);
extern int strcmp(char *s1, char *s2);

#endif



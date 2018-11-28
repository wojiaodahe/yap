#ifndef __PROC_H__
#define __PROC_H__

#define PROCESS_READY		(0)
#define PROCESS_SLEEP		(1 << 0)
#define PROCESS_WAIT_EVENT	(1 << 1)

extern void OS_Sched(void);
extern void process_sleep(unsigned int sec);
extern void process_msleep(unsigned int m);
extern void panic(void);

#endif


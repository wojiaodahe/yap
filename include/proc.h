#ifndef __PROC_H__
#define __PROC_H__

#define PROCESS_READY	    	    (0)
#define PROCESS_SLEEP	    	    (1 << 0)
#define PROCESS_WAIT        	    (1 << 1)
#define PROCESS_WAIT_TIMEOUT        (1 << 2)
#define PROCESS_WAIT_INTERRUPTIBLE  (1 << 3)

extern void OS_Sched(void);
extern void process_sleep(unsigned int sec);
extern void process_msleep(unsigned int m);
extern void panic(void);

#endif


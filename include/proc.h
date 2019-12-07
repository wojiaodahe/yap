#ifndef __PROC_H__
#define __PROC_H__

#define PROCESS_READY	    	    (0)
#define PROCESS_SLEEP	    	    (1 << 0)
#define PROCESS_WAIT        	    (1 << 1)
#define PROCESS_WAIT_TIMEOUT        (1 << 2)
#define PROCESS_WAIT_INTERRUPTIBLE  (1 << 3)

enum
{
    PROCESS_PRIO_HIGHEST = 0,
    PROCESS_PRIO_HIGH,
    PROCESS_PRIO_NORMAL,
    PROCESS_PRIO_LOW,
    PROCESS_PRIO_IDLE,

    PROCESS_PRIO_BUTT,
};

extern void OS_Sched(void);
extern void process_sleep(unsigned int sec);
extern void process_msleep(unsigned int m);
extern void panic(void);
extern unsigned int OS_Get_Kernel_Ticks(void);
extern void OS_Clock_Tick(void *arg);
extern int OS_Init(void);
extern void OS_Start(void);


#define MAX_SCHEDULE_TIMEOUT 0xfffffff

extern void set_current_state(unsigned int state);
extern void set_task_state(void *task, unsigned int state);
extern int kernel_thread_prio(int (*f)(void *), void *args, unsigned int prio);

#endif


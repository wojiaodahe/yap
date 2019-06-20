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


#define MAX_SCHEDULE_TIMEOUT 0xfffffff

void set_current_state(unsigned int state);
void set_task_state(void *task, unsigned int state);

#endif


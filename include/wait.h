#ifndef INCLUDE_WAIT_H_
#define INCLUDE_WAIT_H_

#include "proc.h"
#include "list.h"
#include "spinlock.h"

typedef struct __wait_queue wait_queue_t;
typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int sync, void *key);

struct __wait_queue
{
    void *priv;
    unsigned int flags;
    wait_queue_func_t func;
    spinlock_t lock;
    struct list_head task_list;
};

#define wait_event(wq, condition)\
    do {\
        while (1)\
        {\
            if ((condition))\
            {\
                break;\
            }\
            OS_Sched();\
        }\
    }while (0)

#define __wait_event_interruptible(wq, condition)\
    do {\
        while (1)\
        {\
            prepare_to_wait(wq, PROCESS_WAIT_INTERRUPTIBLE);\
            if ((condition))\
            {\
                break;\
            }\
            OS_Sched();\
        }\
        finish_wait(wq);\
    }while (0)

#define wait_event_interruptible(wq, condition)\
    if (!(condition))\
        __wait_event_interruptible(wq, condition)

#define wait_event_timeout(wq, condition)

#define wake_up(wq) __wake_up(wq)
#define wake_up_interruptible(wq) __wake_up_interruptible(wq)
#define wake_up_timeout(wq)

#define init_waitqueue_head(q) \
    do\
    {\
        __init_waitqueue_head((q), #q);\
    } while (0)

extern void prepare_to_wait(wait_queue_t *wq, unsigned int state);
extern void finish_wait(wait_queue_t *wq);
extern void __wake_up_interruptible(wait_queue_t *wq);
extern void __init_waitqueue_head(wait_queue_t *wq, char *name);
extern void __wake_up(wait_queue_t *wq);

#endif /* INCLUDE_WAIT_H_ */



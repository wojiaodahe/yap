/*
 * wait.h
 *
 *  Created on: 2018Äê6ÔÂ24ÈÕ
 *      Author: crane
 */

#ifndef INCLUDE_WAIT_H_
#define INCLUDE_WAIT_H_

#include "proc.h"
#include "list.h"

typedef struct __wait_queue wait_queue_t;
typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int sync, void *key);

struct __wait_queue
{
	unsigned int flags;
	void *priv;
	wait_queue_func_t func;
	struct list_head task_list;
};


#define wait_event(wq, condition)\
do {\
	while (1)\
	{							\
		if ((condition))						\
		{\
			break;						\
		}\
		OS_Sched();						\
	}								\
}while (0)

#define wake_up(wq) __wake_up(wq)

#endif /* INCLUDE_WAIT_H_ */

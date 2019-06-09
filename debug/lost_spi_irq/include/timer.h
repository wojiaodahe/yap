#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_
#include "list.h"

struct timer_list
{
	unsigned long expires;
	void *data;
	void (*function)(void *);
	struct list_head list;
};

extern void timer_list_process(void);
extern int mod_timer(struct timer_list *timer, unsigned long expires);
extern int add_timer(struct timer_list *timer);
extern void del_timer(struct timer_list *timer);
extern void timer_list_init(void);

#endif /* INCLUDE_TIMER_H_ */

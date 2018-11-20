/*
 * timer.h
 *
 *  Created on: 2018Äê7ÔÂ2ÈÕ
 *      Author: crane
 */

#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_

struct timer_list
{
	unsigned long expires;
	void *data;
	void (*function)(void *);
	struct list_head list;
};


#endif /* INCLUDE_TIMER_H_ */

#include "list.h"
#include "timer.h"
#include "error.h"

static struct list_head timer_list_head;
int add_timer(struct timer_list *timer)
{
	struct list_head *list;
	struct timer_list *tmp;

	if (!timer)
		return -EINVAL;

	list_for_each(list, &timer_list_head)
	{
		tmp = list_entry(list, struct timer_list, list);

		if (tmp == timer)
			return 0;
	}

	list_add_tail(&timer->list, &timer_list_head);

	return 0;
}

void del_timer(struct timer_list *timer)
{
	if (!timer)
		return;

    //disable_irq();

    list_del(&timer->list);
	INIT_LIST_HEAD(&timer->list);

    //enable_irq();
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{
	struct list_head *list;
	struct timer_list *tmp;
    int ret = -1;

	if (!timer)
		return -EINVAL;

    //disable_irq();
    list = timer_list_head.next;
    while (list != &timer_list_head)
	{
		tmp = list_entry(list, struct timer_list, list);
        list = list->next;
		if (tmp == timer)
		{
			tmp->expires = expires;
			ret = 0;
            break;
		}
	}

    //enable_irq();
	return ret;
}

void timer_list_process()
{
	struct list_head *list;
	struct timer_list *timer;

	if (list_empty(&timer_list_head))
		return;

    list = timer_list_head.next;
    while (list != &timer_list_head)
	{
		timer = list_entry(list, struct timer_list, list);
        list = list->next;
		if (timer->expires > 0)
		{
			timer->expires--;
			if(!timer->expires)
				timer->function(timer->data);
		}
	}
}

void timer_list_init()
{
	INIT_LIST_HEAD(&timer_list_head);
}

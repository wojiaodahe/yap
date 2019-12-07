#include "pcb.h"
#include "config.h"
#include "list.h"
#include "printk.h"

typedef struct 
{
	unsigned char sp[TASK_STACK_SIZE];
}stack_t;


pcb_t 	task_pcb[MAX_TASK_NUM];
stack_t	task_sp[MAX_TASK_NUM];
pcb_t *alloc_pcb(void)
{
	static unsigned int cur_pcb = 0;
	
	if (cur_pcb < MAX_TASK_NUM)
	{
		INIT_LIST_HEAD(&task_pcb[cur_pcb].wq.task_list);
		return &task_pcb[cur_pcb++];
	}

	return 0;
}

void *alloc_stack(void)
{

	static unsigned int cur_sp = 0;
	if (cur_sp < MAX_TASK_NUM)
		return (void *)(task_sp[cur_sp++].sp);

	return 0;
}


void pcb_list_add(pcb_t *head, pcb_t *pcb)
{
	pcb_t *tmp = pcb;

    tmp->next = head->next;
    tmp->prev = head;
    head->next->prev = tmp;
    head->next = tmp;
}

pcb_t *pcb_list_init(void)
{
	pcb_t *tmp;

    if ((tmp = (pcb_t *)alloc_pcb()) == (void *)0)
    {
        printk("%s failed\n", __func__);
        panic();
    }

	tmp->next = tmp;
	tmp->prev = tmp;
	tmp->pid = -1;

	return tmp;
}


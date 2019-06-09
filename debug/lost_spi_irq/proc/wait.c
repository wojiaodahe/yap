#include "wait.h"
#include "pcb.h"

extern pcb_t *current;

#if 0
void __wake_up(wait_queue_t *wq)
{
    pcb_t *pcb;

    if (!wq || !wq->priv)
        return;

    pcb = wq->priv;
    pcb->p_flags = PROCESS_READY;
}
#else
void __wake_up(wait_queue_t *wq)
{

}
#endif

void do_wait(wait_queue_t *wq)
{
    OS_Sched();
}

void prepare_to_wait(wait_queue_t *wq, unsigned int state)
{
    wq->priv = current;
    current->p_flags |= state;
}

void finish_wait(wait_queue_t *wq)
{
    pcb_t *pcb;

    if (!wq || !wq->priv)
        return;

    pcb = wq->priv;
    pcb->p_flags = PROCESS_READY;
}

void __wake_up_interruptible(wait_queue_t *wq)
{

    pcb_t *pcb;

    if (!wq || !wq->priv)
        return;

    pcb = wq->priv;
    pcb->p_flags = PROCESS_READY;
}

void __init_waitqueue_head(wait_queue_t *wq, char *name)
{
    spin_lock_init(&wq->lock);
    INIT_LIST_HEAD(&wq->task_list);
}


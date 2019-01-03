#include "wait.h"
#include "pcb.h"

extern pcb_t *current;

void __wake_up(wait_queue_t *wq)
{

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



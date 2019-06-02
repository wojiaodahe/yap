#include "completion.h"

void wait_for_completion(struct completion *x)
{
    //spin_lock_irq();  

    if (!x->done)
        wait_event(&x->wait, x->done);

    if (x->done)
        x->done--;
    //spin_unlock_irq();
}

void complete(struct completion *x)
{
    //spin_lock_irqsave();  
   
    x->done++;
    wake_up(&x->wait);

    //spin_unlock_irqrestore();
}

void init_completion(struct completion *x)
{
    x->done = 0;
    //init_wait_queue_head(&x->wait);
}

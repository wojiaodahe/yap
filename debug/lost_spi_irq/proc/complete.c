#include "completion.h"

void wait_for_completion(struct completion *x)
{
    spin_lock_irq(&x->wq.lock);  

    if (!x->done)
        wait_event(&x->wq, x->done);

    if (x->done)
        x->done--;
   spin_unlock_irq(&x->wq.lock);
}

void complete(struct completion *x)
{
    spin_lock_irqsave(&x->wq.lock);  
   
    x->done++;
    wake_up(&x->wq);

    spin_unlock_irqrestore(&x->wq.lock);
}

void init_completion(struct completion *x)
{
    x->done = 0;
    init_waitqueue_head(&x->wq);
}

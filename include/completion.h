#ifndef __COMPLETION_H__
#define __COMPLETION_H__

#include "wait.h"

struct completion
{   
    unsigned int done;
    wait_queue_t wait;
};


void wait_for_completion(struct completion *x);
void complete(struct completion *x);
void init_completion(struct completion *x);

#endif



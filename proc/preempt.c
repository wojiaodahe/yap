#include "proc.h"
#include "pcb.h"

extern pcb_t *current;

inline void add_preempt_count(unsigned int val)
{   
    current->preempt_count += val;
}

inline void sub_preempt_count(unsigned int val)
{
    if (current->preempt_count >= val)
        current->preempt_count -= val;
    else 
        current->preempt_count = 0;
}

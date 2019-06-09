#ifndef __PREEMPT_H__
#define __PREEMPT_H__

#if 0
#define add_preempt_count(val)\
        do\
        {\
            preempt_count += val;\
        } while (0)

#define sub_preempt_count(val)\
        do\
        {\
            if (preempt_count >= val)\
                preempt_count -= val;\
            else\
                preempt_count = 0;\
        } while (0)
#endif

extern inline void add_preempt_count(unsigned int val);
extern inline void sub_preempt_count(unsigned int val);


#define inc_preempt_count() add_preempt_count(1)
#define dec_preempt_count() sub_preempt_count(1)

#define preempt_disable()\
        do\
        {\
            inc_preempt_count();\
        } while (0)

#define preempt_enable()\
        do\
        {\
            dec_preempt_count();\
        } while (0)

#endif



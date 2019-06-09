#ifndef __BUG_H__
#define __BUG_H__

#include "common.h"

#define BUG_ID_NULL_PTR    0

#define BUG_NULL_PTR(ptr) \
        {\
            printk("BUG: %s %d %s %s is NULL\n", __FILE__, __LINE__, __func__, ptr);\
            bug(BUG_ID_NULL_PTR);\
        }

void bug(int bug_id);

#endif



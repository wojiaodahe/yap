#ifndef __SPINLOCK_TYPES_H__
#define __SPINLOCK_TYPES_H__

#include "preempt.h"
#include "asm/arch_spinlock_types.h"

typedef struct raw_spinlock
{
   arch_spinlock_t raw_lock; 
}raw_spinlock_t;

typedef struct spinlock
{
    struct raw_spinlock rlock;
}spinlock_t;

#endif



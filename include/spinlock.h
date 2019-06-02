#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include "spinlock_types.h"

#define spinlock_check(_lock)

#define spin_lock_init(_lock)                   \
    do                                          \
    {                                           \
        spinlock_check(_lock);                  \
        raw_spin_lock_init(&((_lock)->rlock));  \
    } while (0);                                \

#define spin_lock(lock)         raw_spin_lock(&((lock)->rlock))
#define spin_lock_bh(lock)      raw_spin_lock_bh(&((lock)->rlock))
#define spin_try_lock(lock)     raw_spin_trylock(&((lock)->rlock))
#define spin_lock_irq(lock)     raw_spin_lock_irq(&((lock)->rlock))

#define spin_unlock(lock)       raw_spin_unlock(&((lock)->rlock))
#define spin_unlock_bh(lock)    raw_spin_unlock_bh(&((lock)->rlock))
#define spin_unlock_irq(lock)   raw_spin_unlock_irq(&((lock)->rlock))

#define spin_acquire()


#endif



#include "spinlock.h"

int do_raw_spin_trylock(raw_spinlock_t *lock)
{
    return arch_spin_trylock(&(lock)->raw_lock);
}

void do_raw_spin_lock(raw_spinlock_t *lock)
{
    arch_spin_lock(&lock->raw_lock);
}

void raw_spin_lock(raw_spinlock_t *lock)
{
    preempt_disable(); /* 禁止抢占  */
    spin_acquire();
    do_raw_spin_lock(lock);
}
 
void do_raw_spin_unlock(raw_spinlock_t *lock)
{
    arch_spin_unlock(&lock->raw_lock);
}

void raw_spin_unlock(raw_spinlock_t *lock)
{
    do_raw_spin_unlock(lock);
    preempt_enable(); 
}

void raw_spin_lock_init(raw_spinlock_t *lock)
{
    arch_spin_lock_init(&lock->raw_lock);
}

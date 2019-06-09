#include "arch_spinlock_types.h"

#if __ARM_ARCH__ >= 6
void arch_spin_lock(arch_spinlock_t *lock)
{
    unsigned long tmp;
    __asm__ __volatile__ (
    "1: ldrex %0, [%1]\n"
    "teq %0, #1\n"
    "strexeq %0, %2, [%1]\n"
    "teqeq %0, #0\n"
    "bne 1b"
        : "=&r" (tmp)
        : "r" (&lock->lock), "r" (0)
        :"cc");
}

int arch_spin_trylock(arch_spinlock_t *lock)
{
    unsigned long tmp;

    __asm__ __volatile__ (
    "ldrex %0, [%1]\n"
    "teq %0, #1\n"
    "strexeq %0, %2, [%1]"
        :"=&r" (tmp)
        :"r" (&lock->lock), "r" (0)
        :"cc")

    return (tmp == 0) ? 1 : 0; 
}

void arch_spin_unlock(arch_spinlock_t *lock)
{
    __asm__ __volatile__ (
    "str %1, [%0]\n"
        :
        :"r" (&lock->lock), "r" (1)
        :"cc")
}

#else

void arch_spin_lock(arch_spinlock_t *lock)
{
    lock->lock = 0;
}

int arch_spin_trylock(arch_spinlock_t *lock)
{
    unsigned int oldval = lock->lock;

    lock->lock = 0;

    return oldval > 0;
}

void arch_spin_unlock(arch_spinlock_t *lock)
{
    lock->lock = 1;
}

#endif

void arch_spin_lock_init(arch_spinlock_t *lock)
{
    arch_spin_unlock(lock);
}



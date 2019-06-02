#ifndef __ASM_SPINLOCK_TYPES_H__
#define __ASM_SPINLOCK_TYPES_H__

typedef struct 
{
    volatile unsigned int lock;
}arch_spinlock_t;

void arch_spin_lock(arch_spinlock_t *lock);
int arch_spin_trylock(arch_spinlock_t *lock);
void arch_spin_unlock(arch_spinlock_t *lock);
void arch_spin_lock_init(arch_spinlock_t *lock);

#endif



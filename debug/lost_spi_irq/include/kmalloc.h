#ifndef __KMALLOC_H__
#define __KMALLOC_H__

extern void kfree(void *addr);
extern void *kmalloc(unsigned int size);
extern int system_mm_init(void);
extern void system_mm_destroy(void);
#endif


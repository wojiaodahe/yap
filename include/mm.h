#ifndef __TINYOS_MM_H__
#define __TINYOS_MM_H__

#define PERM_RW					(0x2)
#define PERM_RO					(0x1)
#define PERM_FORBID				(0x0)


#if 0
typedef struct 
{
	unsigned int permission;
}permission_t;
#else
typedef unsigned int permission_t;
#endif

extern int set_mem_access_permission(unsigned long, unsigned long, permission_t, permission_t);
extern int system_mm_init(void);

#endif



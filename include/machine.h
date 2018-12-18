#ifndef __MACHINE_H__
#define __MACHINE_H__

struct sys_timer
{
    void (*init)(void);
    void (*suspend)(void);
    void (*resume)(void);
};

struct machine_desc
{
    unsigned int        nr;
    char                *name;
    void                (*irq_init)(void);
    void                (*machine_init)(void);
    void                (*tty_init)(void);
    struct sys_timer    *sys_timer;
};

#endif


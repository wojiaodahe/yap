#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include "../include/types.h"

/* ipc */
#define SEND		1
#define RECEIVE		2
#define BOTH		3	/* BOTH = (SEND | RECEIVE) */


/**
 * MESSAGE mechanism is borrowed from MINIX
 */
struct mess1 
{
    int m1i1;
    int m1i2;
    int m1i3;
    int m1i4;
};
struct mess2 
{
    void* m2p1;
    void* m2p2;
    void* m2p3;
    void* m2p4;
};
struct mess3 
{
    int	m3i1;
    int	m3i2;
    int	m3i3;
    int	m3i4;
    u64	m3l1;
    u64	m3l2;
    void*	m3p1;
    void*	m3p2;
};
typedef struct 
{
    int source;
    int type;
    union 
	{
        struct mess1 m1;
        struct mess2 m2;
        struct mess3 m3;
    } u;
}MESSAGE;

/* Process */
#define SENDING   0x02	/* set when proc trying to send */
#define RECEIVING 0x04	/* set when proc trying to recv */

#define ANY		    (10)
#define NO_TASK		(10)

#define INTERRUPT	-10

enum msgtype 
{
    /* 
     * when hard interrupt occurs, a msg (with type==HARD_INT) will
     * be sent to some tasks
     */
    HARD_INT = 1,
    GET_TICKS,
};

#define	RETVAL		u.m3.m3i1


#endif 


#ifndef __PCB_H__
#define __PCB_H__
#include "message.h"
#include "fs.h"
#include "config.h"
#include "wait.h"

#define disable_schedule(x)	enter_critical()
#define enable_schedule(x)	exit_critical()

extern void *kmalloc(unsigned int size);

typedef enum 
{
	running,
	ready,
	wait,
	sleep,
}status;

typedef struct _heap_slab
{
	unsigned int heap_slab_start;
	unsigned int heap_slab_size;
	struct _heap_slab *heap_slab_next;
}heap_slab;

typedef struct pcb
{
	unsigned int sp;
	unsigned int sp_size;
	unsigned int sp_bottom;

	int pid;
	char proc_name[16];
	unsigned int p_flags;
	unsigned int sleep_time;

	int status;

	unsigned int prio;
	unsigned int counter;

	unsigned int process_mem_size;
	unsigned int phyaddr;
	unsigned int viraddr;

	unsigned int authority;//进程权限（内核进程、用户进程）

	int time_slice;
	int ticks;

	wait_queue_t wq;

	MESSAGE *p_msg;
	int p_recvfrom;
	int p_sendto;
	struct pcb *q_sending;
	struct pcb *next_sending;
	int has_int_msg;           /**
									* nonzero if an INTERRUPT occurred when
									* the task is not ready to deal with it.
									*/
	int nr_tty;


	struct pcb 		*next;
	struct pcb 		*prev;
	struct file 	*filp[NR_OPEN];
	struct inode	*pwd;
	struct inode 	*root;
}pcb_t;

#endif 


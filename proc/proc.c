#include "head.h"
#include "kernel.h"
#include "interrupt.h"
#include "error.h"
#include "config.h"
#include "proc.h"
#include "wait.h"
#include "timer.h"
#include "syslib.h"
#include "printk.h"
#include "vfs.h"
#include "inet.h"

extern void pcb_list_add(pcb_t *head, pcb_t *pcb);
extern pcb_t * pcb_list_init(void);
extern void __soft_schedule(void);
extern void __int_schedule(void);
extern pcb_t *alloc_pcb(void);
extern void *alloc_stack(void);

pcb_t *next_run;
pcb_t *current;
static struct proc_list_head proc_list[PROCESS_PRIO_BUTT];
static pcb_t sleep_proc_list_head;
static unsigned int OS_TICKS = 0;
static unsigned int pid = 0;
unsigned char OS_RUNNING = 0;
extern unsigned int OSIntNesting;

#define DO_INIT_SP(sp,fn,args,lr,cpsr,pt_base)									\
		do{																		\
				(sp)=(sp)-4;/*r15*/												\
				*(volatile unsigned int *)(sp)=(unsigned int)(fn);/*r15*/		\
				(sp)=(sp)-4;/*cpsr*/											\
				*(volatile unsigned int *)(sp)=(unsigned int)(cpsr);/*r14*/		\
				(sp)=(sp)-4;/*lr*/												\
				*(volatile unsigned int *)(sp)=(unsigned int)(lr);				\
				(sp)=(sp)-4*13;/*r12,r11,r10,r9,r8,r7,r6,r5,r4,r3,r2,r1,r0*/	\
				*(volatile unsigned int *)(sp)=(unsigned int)(args);			\
		}while(0)

void *get_process_sp()
{
	void *tmp;
    
    tmp = (void *)alloc_stack();
    printk("get_sp_addr: %x\r\n", (unsigned int)tmp);
	return tmp;
}

unsigned int get_cpsr(void)
{
	unsigned int p;
	
	__asm
	{
		mrs p, cpsr
	}

	return p;
}

void thread_exit(void)
{
    while (1)
    {
        ssleep(1);
    }
}

int kernel_thread(int (*f)(void *), void *args)
{
	unsigned int sp;
	
	pcb_t *pcb = (pcb_t *)alloc_pcb();
	if (!pcb)
		return -ENOMEM;

	if((sp = (unsigned int)get_process_sp()) == 0)
	{
		printk("kernel_thread get sp_space error\r\n");
		return -ENOMEM;
	}

	printk("get_pcb_addr: %x\r\n", (unsigned int)pcb);

	sp 				= (sp + TASK_STACK_SIZE);
	pcb->sp 		= sp;
//	pcb->sp_bottom  = sp;
	pcb->sp_size 	= TASK_STACK_SIZE;

    pcb->prio       = PROCESS_PRIO_NORMAL;
	pcb->pid 		= pid++;
	pcb->time_slice = 5;
	pcb->ticks 		= 5;
	pcb->root 		= current->root;
	pcb->pwd  		= current->pwd;
	memcpy(pcb->filp, current->filp, sizeof (pcb->filp));
	
	DO_INIT_SP(pcb->sp, f, args, thread_exit, 0x1f & get_cpsr(), 0);

	enter_critical();
	pcb_list_add(&proc_list[pcb->prio].head, pcb);
	exit_critical();

	return 0;
}

int kernel_thread_prio(int (*f)(void *), void *args, unsigned int prio)
{
	unsigned int sp;
	
	pcb_t *pcb = (pcb_t *)alloc_pcb();
	if (!pcb)
		return -ENOMEM;

	if((sp = (unsigned int)get_process_sp()) == 0)
	{
		printk("kernel_thread get sp_space error\r\n");
		return -ENOMEM;
	}

	printk("get_pcb_addr: %x\r\n", (unsigned int)pcb);

	sp 				= (sp + TASK_STACK_SIZE);
	pcb->sp 		= sp;
//	pcb->sp_bottom  = sp;
	pcb->sp_size 	= TASK_STACK_SIZE;

    pcb->prio       = prio;
	pcb->pid 		= pid++;
	pcb->time_slice = 5;
	pcb->ticks 		= 5;
	pcb->root 		= current->root;
	pcb->pwd  		= current->pwd;
	memcpy(pcb->filp, current->filp, sizeof (pcb->filp));
	
	DO_INIT_SP(pcb->sp, f, args, thread_exit, 0x1f & get_cpsr(), 0);

	enter_critical();
	pcb_list_add(&proc_list[prio].head, pcb);
	exit_critical();

	return 0;
}

pcb_t *OS_GetNextReady(void)
{
    int prio;
    pcb_t *tmp;
    
    for (prio = 0; prio < PROCESS_PRIO_BUTT; prio++)
    {
        tmp = proc_list[prio].current;
        
        do
        {
            tmp = tmp->next;
            if (tmp->pid == -1)
                continue;
            if (tmp->p_flags == PROCESS_READY)
            {
                proc_list[prio].current = tmp;
                return tmp;
            }
        } while (tmp != proc_list[prio].current);
    }

    enter_critical();
    printk("No Ready Process!! \n");
    panic();

    return tmp;
}

void OS_IntSched()
{
	
	current = OS_GetNextReady();
    
	__int_schedule();
}

void OS_Sched()
{

    if (!OS_RUNNING)
    {
        printk("OS_Sched Before OS_RUNNING\n");
        panic();
    }
	kernel_disable_irq();
    
    next_run = OS_GetNextReady();
	
	__soft_schedule();
    
    kernel_enable_irq();
}

int OS_SYS_PROCESS(void *p)
{
	int src;
	while (1)
    {
        MESSAGE msg;
        while (1) 
        {
            send_recv(RECEIVE, ANY, &msg);
            src = msg.source;

            switch (msg.type) {
            case GET_TICKS:
                msg.RETVAL = 666;
                send_recv(SEND, src, &msg);
                break;
            default:
   //             panic("unknown msg type");
                break;
            }
        }
	}
	//return 0;
}

int OS_IDLE_PROCESS(void *arg)
{
	while (1)
	{
		printk("OS Idle Process\r\n");
		OS_Sched();
	}
}

void print_sleep_list(void)
{
    pcb_t *tmp;

    tmp = sleep_proc_list_head.next_sleep_proc;

    while (tmp != &sleep_proc_list_head)
    {
        printk("pid: %d sleep_time: %d\n", tmp->pid, tmp->sleep_time);
        tmp = tmp->next_sleep_proc;
    }
}

void add_current_to_sleep_list(void)
{
   sleep_proc_list_head.next_sleep_proc->prev_sleep_proc = current;
   
   current->next_sleep_proc = sleep_proc_list_head.next_sleep_proc;
   current->prev_sleep_proc = &sleep_proc_list_head;
  
   sleep_proc_list_head.next_sleep_proc = current;
}

void process_sleep(unsigned int sec)
{
	enter_critical();

	current->p_flags |= PROCESS_SLEEP;
	current->sleep_time = sec * HZ;

    add_current_to_sleep_list();

	exit_critical();
	OS_Sched();
}

void process_msleep(unsigned int m)
{
	enter_critical();

	current->p_flags |= PROCESS_SLEEP;
	current->sleep_time = m * HZ / 1000;
	if (!current->sleep_time)
		current->sleep_time = 1;
    
    add_current_to_sleep_list();

	exit_critical();
	OS_Sched();
}

void set_task_status(unsigned int status)
{
	enter_critical();

	current->p_flags |= status;

	exit_critical();
}

void clr_task_status(unsigned int status)
{
	enter_critical();

	current->p_flags &= ~(status);

	exit_critical();
}

void update_sleeping_proc(void)
{
    pcb_t *tmp;
	
    tmp = sleep_proc_list_head.next_sleep_proc;
	while (tmp != &sleep_proc_list_head)
	{
        if (tmp->sleep_time > 0)
            tmp->sleep_time--;
        if (tmp->sleep_time == 0)
        {
            tmp->p_flags &= ~(PROCESS_SLEEP);
            tmp->next_sleep_proc->prev_sleep_proc = tmp->prev_sleep_proc; 
            tmp->prev_sleep_proc->next_sleep_proc = tmp->next_sleep_proc;
        }
        tmp = tmp->next_sleep_proc;
	}

}

void OS_Clock_Tick(void *arg)
{
    //
    OS_TICKS++;
    //

    timer_list_process();
    update_sleeping_proc();

    if (current->ticks > 0)	
        current->ticks--;
   
    if (current->ticks > 0)
        return;

    current->ticks = current->time_slice;
    
    OSIntNesting--;
    OS_IntSched();

    /*OSIntNesting-- 不能放在这里，因为代码不会执行到这儿 */
}

unsigned int OS_Get_Kernel_Ticks(void)
{
    return OS_TICKS;
}
		
void panic(void)
{
	while (1)
		;
}

extern int test_user_syscall_open(void *argc);
extern int  test_nand(void *p);
extern int  test_get_ticks(void *p);
extern int  test_open_led0(void *p);
extern int  test_open_led1(void *p);
extern int  test_open_led2(void *p);
extern int  test_open_led3(void *p);
extern int test_user_syscall_printf(void *argc);
extern int test_wait_queue(void *p);
extern int test_socket(void *p);
extern int led_driver_init(void);
extern int led_device_init(void);
extern void LWIP_INIT(void);
extern int socket_init(void);
extern int create_stdin_stdout_stderr_device(void);
extern void bus_list_init(void);
extern int platform_bus_init(void);
extern int test_exit(void *arg);
int OS_INIT_PROCESS(void *argv)
{
	int ret;
	int fd_stdin;
	int fd_stdout;
	int fd_stderr;

	enter_critical();
	sys_timer_init();

	fd_stdout = sys_open("/dev/stdout", 0, 0);
	if (fd_stdout < 0)
	{
		printk("Init Process Open stdout device failed\n");
		panic();
	}

	fd_stdin = sys_open("/dev/stdin", 0, 0);
	if (fd_stdin < 0)
	{
		printk("Init Process Open stdin device failed\n");
		panic();
	}

	fd_stderr = sys_open("/dev/stderr", 0, 0);
	if (fd_stderr < 0)
	{
		printk("Init Process Open stderr device failed\n");
		panic();
	}

#if 1

#if 0
	ret = kernel_thread(OS_SYS_PROCESS,  (void *)0);
	if (ret < 0)
	{
		printk("create OS_SYS_PROCESS error\n");
		panic();
	}
#endif

	ret = kernel_thread_prio(OS_IDLE_PROCESS, (void *)0, PROCESS_PRIO_IDLE);
	if (ret < 0)
	{
		printk("create OS_IDLE_PROCESS error\n");
		panic();
	}
	
	//led_module_init();
	led_driver_init();
	led_device_init();
	//key_module_init();

	socket_init();
	net_core_init();
	dm9000_module_init();

	//create_pthread(test_get_ticks, (void *)1, 10);
	kernel_thread(test_open_led0, (void *)2);
	kernel_thread(test_open_led1, (void *)2);
	kernel_thread(test_open_led2, (void *)2);
	kernel_thread(test_open_led3, (void *)2);
//	kernel_thread(test_nand, (void *)2);
//	kernel_thread(test_user_syscall_open, (void *)2);
	kernel_thread_prio(test_user_syscall_printf, (void *)2, PROCESS_PRIO_HIGH);
//	kernel_thread(test_wait_queue, (void *)2);
	kernel_thread(test_socket, (void *)2);
//	kernel_thread_prio(test_exit,   (void *)2, PROCESS_PRIO_LOW);
    
    OS_RUNNING = 1;
    exit_critical();

#endif
	while (1)
	{
		//printk("OS Init Process\r\n");
		OS_Sched();
	}
}

int OS_Init(void)
{
    int prio;
	int ret;

	current = &proc_list[PROCESS_PRIO_NORMAL].head;

    sleep_proc_list_head.pid = -1;
    sleep_proc_list_head.next_sleep_proc = &sleep_proc_list_head;
    sleep_proc_list_head.prev_sleep_proc = &sleep_proc_list_head;

    for (prio = 0; prio < PROCESS_PRIO_BUTT; prio++)
    {
        proc_list[prio].head.pid = -1;
        proc_list[prio].head.next = &proc_list[prio].head;
        proc_list[prio].head.prev = &proc_list[prio].head;
        proc_list[prio].current = &proc_list[prio].head;
    }
	
	ret = system_mm_init();
	if (ret < 0)
	{
		printk("system kmalloc init error\n");
		panic();
	}
	
    s3c24xx_init_irq();
	s3c24xx_init_tty();
	
    ret = vfs_init();
	if (ret < 0)
	{
		printk("vfs init error\n");
		panic();
	}

	ret = sys_mkdir("/dev", 0);
	if (ret < 0)
	{
		printk("/dev directory error\n");
		panic();
	}
	
	ret = create_stdin_stdout_stderr_device();
	if (ret < 0)
	{
		printk("create_std_device error\n");
		panic();
	}

	bus_list_init();
    ret = platform_bus_init();
    if (ret < 0)
    {
    	printk("platform bus init failed");
    	panic();
    }

	ret = kernel_thread(OS_INIT_PROCESS, (void *)0);
	if (ret < 0)
	{
		printk("create OS_INIT_PROCESS error\n");
		panic();
	}
	
	current = proc_list[PROCESS_PRIO_NORMAL].head.next;

	timer_list_init();
    
    return 0;
}

int proc2pid(pcb_t *proc)
{
    return proc->pid;
}

pcb_t *pid2proc(int pid)
{

    enter_critical();
    
    printk("Uncomplete Function %s\n", __func__);
   
    panic();

    return 0;
}

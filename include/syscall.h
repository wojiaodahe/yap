#ifndef __OS_SYSCALL_H__
#define __OS_SYSCALL_H__

#define  OS_GET_TICKS   	1
#define  OS_SEND_RECV   	2
#define  SYSTEM_CALL_OPEN 	3
#define	 SYSTEM_CALL_READ	4
#define	 SYSTEM_CALL_WRITE	5
#define	 SYSTEM_CALL_CLOSE	6
#define	 SYSTEM_CALL_IOCTL	7
#define	 SYSTEM_CALL_LSEEK	8
#define  SYSTEM_CALL_PRINTF	20
#define  SYSTEM_CALL_SSLEEP	21
#define  SYSTEM_CALL_MSLEEP	22

struct system_call_tag
{
	unsigned int id;
	int (*fun)(int , int *);
};

extern void ssleep(unsigned int time);
extern void msleep(unsigned int time);
extern int myprintf(char *fmt, ...);
extern unsigned int OS_Get_Ticks(void);
extern int syscall(int argc, unsigned long *argv);


#endif 


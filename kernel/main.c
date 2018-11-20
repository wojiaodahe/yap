#include "head.h"
#include "fs.h"
#include "syscall.h"

#include "unistd.h"
#include "fcntl.h"
#include "wait.h"
#include "socket.h"
#include "inet_socket.h"

int  test_get_ticks(void *p)
{
	int t;
	while (1)
    {	
		printk("Process1 GetTicks\r\n");
		t = OS_Get_Ticks();
        printk("Process1 GetTicks: %d\r\n", t);
        OS_Sched();
	}

	//return 0;
}

int test_wait_queue(void *p)
{
	int fd;
	char buf[64];

	fd = sys_open("/key", 0, 0);
	while (1)
	{
		memset(buf, 0, 64);
		sys_read(fd, buf, 64);
		printk("++++++++++++++++wait_queue read++++++++++++++++++\n");
	}
	return 0;
}

int test_socket(void *p)
{
	unsigned int ip = 0xc0a80107;
	while (1)
	{
		//arp_send_request(htonl(ip), return_ndev());
		//frag_test();
		test_udp_send();
		ssleep(1);
	}
}

int  test_open_led0(void *p)
{
	int fd;

	fd = sys_open("led0", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
		ssleep(1);
		sys_ioctl(fd, 1, 1);
		ssleep(1);
	}

	return 0;
}

int  test_open_led1(void *p)
{
	int fd;

	fd = sys_open("led1", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
		msleep(500);
		sys_ioctl(fd, 1, 1);
		msleep(500);
	}

	return 0;
}

int  test_open_led2(void *p)
{
	int fd;

	fd = sys_open("led2", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
		msleep(250);
		sys_ioctl(fd, 1, 1);
		msleep(250);
	}
	return 0;
}

int  test_open_led3(void *p)
{
	int fd;

	fd = sys_open("led3", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
		msleep(125);
		sys_ioctl(fd, 1, 1);
		msleep(125);
	}

	return 0;
}

char buff[3096];
int  test_nand(void *p)
{
	int i;
	int fd;
	int ret;

	if ((fd = sys_open("/nand/abc/test_ofs.c", 0)) < 0)
	{
		printk("open error");
	}

	while (1)
	{
		ret = sys_read(fd, buff, 2555);
		if (ret <= 0)
			break;
		for (i = 0; i < ret; i++)
		{
			printk("%c", buff[i]);
		}
	}
	while (1)
    {
		OS_Sched();
	}
	//return 0;
}

int test_user_syscall_open(void *argc)
{
	int fd;
	int ret, i;
	fd = open("/nand/abc/test_ofs.c", 0, 0);
	
	while (1)
	{
		ret = read(fd, buff, 2555);
		if (ret <= 0)
			break;
		for (i = 0; i < ret; i++)
		{
			printk("%c", buff[i]);
		}
	}

	while (1)
	{
		OS_Sched();
	}
}

int test_user_syscall_printf(void *argc)
{
	while (1)
	{
		//myprintf("Process Test Printf %d %x %c %s", 10, 0xaa, 'p', "test string\n");
		ssleep(1);
	}
}

int kernel_main()
{
	s3c24xx_init_tty();
    init_key_irq();
	
	init_user_program_space();
    
	OS_Init();
	test_platform();
	OS_Start();

	while (1)
	{
		
	}
//    return 0;
}

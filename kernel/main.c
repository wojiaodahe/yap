#include "head.h"
#include "fs.h"
#include "syscall.h"

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

int  test_open_led(void *p)
{
	int fd;
	fd = sys_open("led", 0, 0);
	while (1)
    {
		sys_ioctl(fd, 0, 0);
	}
	//return 0;
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
	DM9000_init();
	while (1)
	{
		myprintf("Process Test Printf %d %x %c %s", 10, 0xaa, 'p', "test string\n");
		TestDm9000();
		ssleep(1);
	}
}

int kernel_main()
{
	int i;
	
	s3c24xx_init_tty();
    init_key_irq();
	
	init_user_program_space();
    
	OS_Init();

	OS_Start();

	while (1)
	{
		
	}
//    return 0;
}

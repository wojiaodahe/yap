#include "fs.h"
#include "syscall.h"
#include "vfs.h"
#include "kmalloc.h"
#include "lib.h"
#include "printk.h"
#include "list.h"

#include "unistd.h"
#include "fcntl.h"
#include "wait.h"
#include "socket.h"
#include "inet.h"
#include "inet_socket.h"
#include "timer.h"
#include "completion.h"

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
}

#define BUFF_SIZE   8192
int test_tcp(void *p)
{
    int ret = 0;
    int fd, new_fd;
    char *buff;
    struct sockaddr_in seraddr;
    struct sockaddr_in cliaddr;
    int addrlen;
	
    seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(8000);
	seraddr.sin_addr.addr = htonl(0xc0a8080a);

    fd = sys_socket(AF_INET, SOCK_STREAM, 0);

    ret = sys_bind(fd, (struct sockaddr *)&seraddr, sizeof (seraddr));
    if (ret < 0)
        printk("bind faild\n");

    ret = sys_listen(fd, 10);
    if (ret < 0)
        printk("sys_listen faild\n");

    new_fd = sys_accept(fd, (struct sockaddr *)&cliaddr, &addrlen);
    
    buff = kmalloc(BUFF_SIZE);
    if (!buff)
    {
        printk("kmalloc failed\n");
        while (1)
            ssleep(100);
    }

#if 0
    int i;
    while (1)
    {
        strcpy(buff, "hello world && hello tcp");
        ret = sys_send(new_fd, buff, strlen(buff), 0);
        printk("sys_send: %d\n", ret);

        memset(buff, 0, 32);
        ret = sys_recv(new_fd, buff, 32, 0);
        printk("sys_recv: %d\n", ret);
        for (i = 0; i < ret; i++)
            printk("%c", buff[i]);
    }
#else
    
    while (1)
    {
        memset(buff, 0, BUFF_SIZE);
        ret = sys_recv(new_fd, buff, BUFF_SIZE, 0);
        //printk("sys_recv: %d\n", ret);
# if 0
        unsigned int *ptr;
        ptr = (unsigned int *)buff;
        for (i = 0; i < ret / 4; i++)
            printk("%d \n", *ptr++);
#endif
    }
#endif

#if 0
	while (1)
	{
        ssleep(100);
	}
#endif

}

int test_socket(void *p)
{
    int i;
    int ret = 0;
    int fd;
    char buff[16];
    struct sockaddr_in seraddr;
    int addr_len;
	
    seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(8000);
	seraddr.sin_addr.addr = htonl(0xc0a80168);
    test_tcp((void *)0);

    strcpy(buff, "helloworld");
    fd = sys_socket(AF_INET, SOCK_DGRAM, 0);
	while (1)
	{
      //  ret = sys_sendto(fd, buff, 20, 0, (struct sockaddr *)&seraddr, sizeof(seraddr));
      //  if (ret < 0)
     //       printk("sys_sendto failed: %d\n", ret);
       
        ret =  sys_recvfrom(fd, buff, 10, 0, (struct sockaddr *)&seraddr, &addr_len);
        printk("sys_recvfrom ret: %d\n", ret);

        for (i = 0; i < ret; i++)
            printk("%c", buff[i]);
        printk("\n");
		
        //ssleep(1);
	}
}


void test_connect(void)
{
    int ret;
	int sockfd;
	struct sockaddr_in seraddr;
	
	if ((sockfd = sys_socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printk("sys_socket failed ret: %d\n");
	
	
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(8000);
	seraddr.sin_addr.addr = htonl(0xc0a80168);
	ret = sys_connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
    if (ret < 0)
        printk("sys_conect failed ret: %d\n");

    sys_send(sockfd, "hello", 5, 0);
    while (1)
    {
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
}

char buff[3096];
int  test_nand(void *p)
{
	int i;
	int fd;
	int ret;

	if ((fd = sys_open("/nand/abc/test_ofs.c", 0, 0)) < 0)
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
		OS_Sched();

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
		OS_Sched();
}

int test_user_syscall_printf(void *argc)
{
	while (1)
	{
//		myprintf("Process Test Printf %d %x %c %s", 10, 0xaa, 'p', "test string\n");
		ssleep(1);
	}
}

int test_exit(void *arg)
{
    printk("thread exit!\n");
    while (1)
        ;
}

void test_completion_func(void *x)
{
    complete(x);
}

struct completion test_done;
struct timer_list test_completion_timer = 
{
    .expires = 100,
    .data = &test_done,
    .function = test_completion_func
};

int test_completion(void *arg)
{
    init_completion(&test_done);
    add_timer(&test_completion_timer);
    printk("&test_completion_timer: %x\n", &test_completion_timer);
    printk("&test_done: %x\n", &test_done);
    while (1)
    {
        mod_timer(&test_completion_timer, 100);
//        printk("Test completion test_done.done %d\n", test_done.done);
        wait_for_completion(&test_done);
    }
}

extern int s3c24xx_init_tty(void);
extern void init_user_program_space(void);
extern int test_platform(void);
int kernel_main()
{
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

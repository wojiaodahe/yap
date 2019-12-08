#include "oledfont.h"
#include "head.h"
#include "fs.h"
#include "syscall.h"
#include "vfs.h"
#include "unistd.h"
#include "fcntl.h"
#include "wait.h"
#include "socket.h"
#include "timer.h"
#include "completion.h"
#include "printk.h"

#define OLED_CMD_INIT       0x100001
#define OLED_CMD_CLEAR_ALL  0x100002
#define OLED_CMD_CLEAR_PAGE 0x100003
#define OLED_CMD_SET_POS    0x100004


void OLEDPutChar(int fd, int page, int col, char c)
{
    /* 得到字模 */
    char *dots = oled_asc2_8x16[c - ' '];

    /* 发给OLED */
    //OLEDSetPos(page, col);
    //sys_ioctl(fd, OLED_CMD_CLEAR_PAGE, page);
    sys_ioctl(fd, OLED_CMD_SET_POS, page | (col << 8));
    /* 发出8字节数据 */
    //for (i = 0; i < 8; i++)
    //    OLEDWriteDat(dots[i]);
    sys_write(fd, &dots[0], 8);

    //OLEDSetPos(page+1, col);
    //sys_ioctl(fd, OLED_CMD_CLEAR_PAGE, page+1);
    sys_ioctl(fd, OLED_CMD_SET_POS, (page+1) | (col << 8));
    /* 发出8字节数据 */
    //for (i = 0; i < 8; i++)
    //    OLEDWriteDat(dots[i+8]);
    sys_write(fd, &dots[8], 8);
}



/* page: 0-7
 * col : 0-127
 * 字符: 8x16象素
 */
void OLEDPrint(int fd, int page, int col, char *str)
{
    int i = 0;

    sys_ioctl(fd, OLED_CMD_CLEAR_PAGE, page);
    sys_ioctl(fd, OLED_CMD_CLEAR_PAGE, page+1);
    while (str[i])
    {
        OLEDPutChar(fd, page, col, str[i]);
        col += 8;
        if (col > 127)
        {
            col = 0;
            page += 2;
            sys_ioctl(fd, OLED_CMD_CLEAR_PAGE, page);
            sys_ioctl(fd, OLED_CMD_CLEAR_PAGE, page + 1);
        }
        i++;
    }
}

void print_usage(char *cmd)
{
    printk("Usage:\n");
    printk("%s init\n", cmd);
    printk("%s clear\n", cmd);
    printk("%s clear <page>\n", cmd);
    printk("%s <page> <col> <string>\n", cmd);
    printk("eg:\n");
    printk("%s 2 0 100ask.taobao.com\n", cmd);
    printk("page is 0,1,...,7\n");
    printk("col is 0,1,...,127\n");
}

int test_oled1(void *arg)
{
    int fd;
    
    fd = open("/dev/oled", 0, 0);
    if (fd < 0)
    {
        printk("can't open /dev/oled\n");
    }

    ssleep(30);
    while (1)
    {
//        OLEDPrint(fd, 4, 0, "1234567890");
        msleep(100);
    }
//    return 0;
}



int test_oled(void *arg)
{
    int i = 0;
    int fd;
    char buf[32];

    //while (1)
     //   ssleep(100);

    fd = sys_open("/dev/oled", 0, 0);
    if (fd < 0)
    {
        printk("can't open /dev/oled\n");
    }

    sys_ioctl(fd, OLED_CMD_INIT, 0);
    sys_ioctl(fd, OLED_CMD_CLEAR_ALL, 0);

    while (1)
    {
        msleep(50);
        sprintk(buf, "OLED: %d", i++);
        OLEDPrint(fd, 2, 0, buf);
        OLEDPrint(fd, 0, 0, "hello world");
    }
    
    //return 0;
}



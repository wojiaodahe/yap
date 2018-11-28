#include "serial_core.h"
#include "tty.h"

#include "s3c24xx.h"
#include "uart.h"
#include "interrupt.h"
#include "kernel.h"
#include "common.h"
#include "error.h"


#define CONFIG_UART_S3C24XX_COUNT			1
#define S3C24XX_UART_MAJOR					204
#define S3C24XX_UART_MINOR					64
#define S3C24XX_UART_TTY_NAME				"ttySAC"

#define TXD0READY   (1 << 2)
#define RXD0READY   (1)

#define PCLK            50000000    // init_system.c中的init_clock函数设置PCLK为50MHz
#define UART_CLK        PCLK        //  UART0的时钟源设为PCLK
#define UART_BAUD_RATE  115200      // 波特率
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)



static struct tty 			s3c24xx_uart_tty;
static struct uart_port		s3c24xx_uart_port;
static struct uart_driver 	s3c24xx_uart_drv;


static void s3c24xx_uart_putchar(struct uart_port *port, int c)
{
	char tmp = c;
   
    while (!(UTRSTAT0 & TXD0READY));

    UTXH0 = tmp;
}

static int s3c24xx_uart_tty_write(struct tty *tty, const char *s, unsigned int count)
{
	int i;
	struct uart_port *port;

	if (!tty)
		return -ENODEV;

	port = (struct uart_port *)(tty->data);
	if (!port)
		return -ENODEV;

	for (i = 0; i < count; i++)
	{
		if (*(s + i) == '\n')
			s3c24xx_uart_putchar(port, '\r');
		s3c24xx_uart_putchar(port, *(s + i));
	}

	return 0;
}

#if 0
static unsigned char s3c24xx_uart_getchar(struct uart_port *port)
{
    /* 等待，直到接收缓冲区中的有数据 */
    while (!(UTRSTAT0 & RXD0READY));

    /* 直接读取URXH0寄存器，即可获得接收到的数据 */
    return URXH0;
}

static int s3c24xx_uart_tty_read(struct tty *tty, char *s, unsigned int count)
{
	int i;
	struct uart_port *port;

	if (!tty)
		return -ENODEV;

	port = (struct uart_port *)(tty->data);
	if (!port)
		return -ENODEV;

	for (i = 0; i < count; i++)
		*s++ = s3c24xx_uart_getchar(port);

	return 0;
}
#endif

void uart_isr(void *arg)
{
   if (SUBSRCPND & (1 << 0)) 
   {
       SUBSRCPND |= (1 << 0);
   }

   if (SUBSRCPND & (1 << 1)) 
   {
       SUBSRCPND |= (1 << 1);
   }
}


/*
 * 初始化UART
 * 115200,8N1,无流控
 */
int init_s3c24xx_uart()
{
    GPHCON  |= 0xa0;    // GPH2,GPH3用作TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3内部上拉

    ULCON0  = 0x03;     // 8N1(8个数据位，无较验，1个停止位)
    UCON0   = 0x05;     // 查询方式，UART时钟源为PCLK
    UFCON0  = 0x00;     // 不使用FIFO
    UMCON0  = 0x00;     // 不使用流控
    UBRDIV0 = UART_BRD; // 波特率为115200

    return put_irq_handler(OS_IRQ_UART_0, uart_isr, &s3c24xx_uart_drv);

}

int s3c24xx_init_tty()
{
	int ret;
	
	s3c24xx_uart_drv.dev_name 	= "s3c24xx_uart";
	s3c24xx_uart_drv.nr 	  	= CONFIG_UART_S3C24XX_COUNT,
	s3c24xx_uart_drv.tty		= &s3c24xx_uart_tty;
	s3c24xx_uart_drv.major		= S3C24XX_UART_MAJOR;

	s3c24xx_uart_tty.name	 	= S3C24XX_UART_TTY_NAME;
	s3c24xx_uart_tty.write		= s3c24xx_uart_tty_write;
	s3c24xx_uart_tty.index		= 0;
	s3c24xx_uart_tty.data		= &s3c24xx_uart_port;

	ret = init_s3c24xx_uart();
	if (ret < 0)
		return ret;

	return register_tty(&s3c24xx_uart_tty);
}



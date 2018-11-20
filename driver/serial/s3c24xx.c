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

#define PCLK            50000000    // init_system.c涓殑init_clock鍑芥暟璁剧疆PCLK涓�50MHz
#define UART_CLK        PCLK        //  UART0鐨勬椂閽熸簮璁句负PCLK
#define UART_BAUD_RATE  115200      // 娉㈢壒鐜�
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

static int s3c24xx_uart_tty_write(struct tty *tty, char *s, unsigned int count)
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

static unsigned char s3c24xx_uart_getchar(struct uart_port *port)
{
    /* 绛夊緟锛岀洿鍒版帴鏀剁紦鍐插尯涓殑鏈夋暟鎹� */
    while (!(UTRSTAT0 & RXD0READY));

    /* 鐩存帴璇诲彇URXH0瀵勫瓨鍣紝鍗冲彲鑾峰緱鎺ユ敹鍒扮殑鏁版嵁 */
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

void uart_isr()
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
 * 鍒濆鍖朥ART
 * 115200,8N1,鏃犳祦鎺�
 */
void init_s3c24xx_uart()
{
    GPHCON  |= 0xa0;    // GPH2,GPH3鐢ㄤ綔TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3鍐呴儴涓婃媺

    ULCON0  = 0x03;     // 8N1(8涓暟鎹綅锛屾棤杈冮獙锛�1涓仠姝綅)
    UCON0   = 0x05;     // 鏌ヨ鏂瑰紡锛孶ART鏃堕挓婧愪负PCLK
    UFCON0  = 0x00;     // 涓嶄娇鐢‵IFO
    UMCON0  = 0x00;     // 涓嶄娇鐢ㄦ祦鎺�
    UBRDIV0 = UART_BRD; // 娉㈢壒鐜囦负115200

    put_irq_handler(OS_IRQ_UART_0, uart_isr, 0);

}

int s3c24xx_init_tty()
{
	s3c24xx_uart_drv.dev_name 	= "s3c24xx_uart";
	s3c24xx_uart_drv.nr 	  	= CONFIG_UART_S3C24XX_COUNT,
	s3c24xx_uart_drv.tty		= &s3c24xx_uart_tty;
	s3c24xx_uart_drv.major		= S3C24XX_UART_MAJOR;

	s3c24xx_uart_tty.name	 	= S3C24XX_UART_TTY_NAME;
	s3c24xx_uart_tty.write		= s3c24xx_uart_tty_write;
	s3c24xx_uart_tty.index		= 0;
	s3c24xx_uart_tty.data		= &s3c24xx_uart_port;

	init_s3c24xx_uart();

	return register_tty(&s3c24xx_uart_tty);
}



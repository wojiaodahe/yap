#ifndef __S3C24XX_IRQ_H__
#define __S3C24XX_IRQ_H__

#define IRQ_EINT0       (0)
#define IRQ_EINT1       (1)
#define IRQ_EINT2       (2)
#define IRQ_EINT3       (3)
#define IRQ_EINT4t7     (4)
#define IRQ_EINT8t23    (5)

#define IRQ_TIMER0      (10)
#define IRQ_TIMER1      (11)
#define IRQ_TIMER2      (12)
#define IRQ_TIMER3      (13)
#define IRQ_TIMER4      (14)

#define IRQ_SDI         (21)
#define IRQ_SPI0        (22)

#define IRQ_UART0       (28)

#define IRQ_SPI1        (29)
#define IRQ_RTC         (30)
#define IRQ_ADC         (31)
#define IRQ_EINT4       (32)
#define IRQ_EINT5       (33)
#define IRQ_EINT6       (34)
#define IRQ_EINT7       (35)
/*.
 *.
 *.
 */
#define IRQ_EINT23      (51)

#define EXTINT_OFFSET   (IRQ_EINT4 - 4)

#endif



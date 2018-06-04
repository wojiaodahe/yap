#include "s3c24xx.h"
/*
 * K1,K2,K3,K4对应GPF1、GPF4、GPF2、GPF0
 */
#define GPF0_int    (0x2 << (0 * 2))
#define GPF1_int    (0x2 << (1 * 2))
#define GPF2_int    (0x2 << (2 * 2))
#define GPF4_int    (0x2 << (4 * 2))

#define GPF0_msk    (3 << (0 * 2))
#define GPF1_msk    (3 << (1 * 2))
#define GPF2_msk    (3 << (2 * 2))
#define GPF4_msk    (3 << (4 * 2))


/*
 * 初始化GPIO引脚为外部中断
 * GPIO引脚用作外部中断时，默认为低电平触发、IRQ方式(不用设置INTMOD)
 */
void init_key_irq(void)
{
    // K1,K2,K3,K4对应的4根引脚设为中断功能
    GPFCON &= ~(GPF0_msk | GPF1_msk | GPF2_msk | GPF4_msk);
    GPFCON |= GPF0_int | GPF1_int | GPF2_int | GPF4_int;

    // 对于EINT4，需要在EINTMASK寄存器中使能它
    EINTMASK &= ~(1 << 4);
    /*
     * 设定优先级：
     * ARB_SEL0 = 00b, ARB_MODE0 = 0: REQ1 > REQ2 > REQ3，即EINT0 > EINT1 > EINT2
     * 仲裁器1、6无需设置
     * 最终：
     * EINT0 > EINT1> EINT2 > EINT4 即K4 > K1 > K3 > K2
     */
    PRIORITY = (PRIORITY & ((~0x01) | ~(0x3 << 7)));

    // EINT0、EINT1、EINT2、EINT4_7使能
    INTMSK &= (~(1 << 0)) & (~(1 << 1)) & (~(1 << 2)) & (~(1 << 4));
}

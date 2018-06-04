#include "s3c24xx.h"
/*
 * K1,K2,K3,K4��ӦGPF1��GPF4��GPF2��GPF0
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
 * ��ʼ��GPIO����Ϊ�ⲿ�ж�
 * GPIO���������ⲿ�ж�ʱ��Ĭ��Ϊ�͵�ƽ������IRQ��ʽ(��������INTMOD)
 */
void init_key_irq(void)
{
    // K1,K2,K3,K4��Ӧ��4��������Ϊ�жϹ���
    GPFCON &= ~(GPF0_msk | GPF1_msk | GPF2_msk | GPF4_msk);
    GPFCON |= GPF0_int | GPF1_int | GPF2_int | GPF4_int;

    // ����EINT4����Ҫ��EINTMASK�Ĵ�����ʹ����
    EINTMASK &= ~(1 << 4);
    /*
     * �趨���ȼ���
     * ARB_SEL0 = 00b, ARB_MODE0 = 0: REQ1 > REQ2 > REQ3����EINT0 > EINT1 > EINT2
     * �ٲ���1��6��������
     * ���գ�
     * EINT0 > EINT1> EINT2 > EINT4 ��K4 > K1 > K3 > K2
     */
    PRIORITY = (PRIORITY & ((~0x01) | ~(0x3 << 7)));

    // EINT0��EINT1��EINT2��EINT4_7ʹ��
    INTMSK &= (~(1 << 0)) & (~(1 << 1)) & (~(1 << 2)) & (~(1 << 4));
}

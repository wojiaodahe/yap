/*
 *
 * ����ϵͳ��ʼ��ʱ���õ� ���� �� ��
 *
 */
#ifndef __SYSTEM_H__
#define __SYSTEM_H_

/* SDRAM regisers */
#define     MEM_CTL_BASE    0x48000000
#define     SDRAM_BASE      0x30000000


/*
 * mmu �ĳ�ʼ��
 * ���ڶ���������һЩ�궨��
 */
#define MMU_FULL_ACCESS     (3 << 10)   /* ����Ȩ�� */
#define MMU_DOMAIN          (0 << 5)    /* �����ĸ��� */
#define MMU_SPECIAL         (1 << 4)    /* ������1 */
#define MMU_CACHEABLE       (1 << 3)    /* cacheable */
#define MMU_BUFFERABLE      (1 << 2)    /* bufferable */
#define MMU_SECTION         (2)         /* ��ʾ���Ƕ������� */
#define MMU_SECDESC         (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | MMU_SECTION)
#define MMU_SECDESC_WB      (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | MMU_CACHEABLE | MMU_BUFFERABLE | MMU_SECTION)
#define MMU_SECDESC_DIS     ((1 << 10) | MMU_DOMAIN | MMU_SPECIAL | MMU_CACHEABLE | MMU_BUFFERABLE | MMU_SECTION)
#define MMU_SECDESC_WB_NCNB (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | MMU_SECTION)
#define MMU_SECTION_SIZE    0x00100000

/*
 * ʱ�ӳ�ʼ������
 */
#define S3C2410_MPLL_200MHZ     ((0x5c << 12) | (0x04 << 4) | (0x00))
#define S3C2440_MPLL_200MHZ     ((0x5c << 12) | (0x01 << 4) | (0x02))

#endif

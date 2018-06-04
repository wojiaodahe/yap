#include "s3c24xx.h"
#include "system.h"
#include "config.h"

extern void enable_irq(void);
extern void disable_irq(void);
extern void umask_int(unsigned int);
extern void usubmask_int(unsigned int offset);



/*
 * 关闭WATCHDOG，否则CPU会不断重启
 */
void disable_watch_dog(void)
{
    WTCON = 0;  // 关闭WATCHDOG很简单，往这个寄存器写0即可
}


void init_memory(void)
{
    int i = 0;
    volatile unsigned long *p = (volatile unsigned long *)MEM_CTL_BASE;

    /* SDRAM 13个寄存器的值 */
    unsigned long  const    mem_cfg_val[] = {
					      0x22011110,     //BWSCON
					      0x00000700,     //BANKCON0
					      0x00000700,     //BANKCON1
					      0x00000700,     //BANKCON2
					      0x00000700,     //BANKCON3
					      0x00000700,     //BANKCON4
					      0x00000700,     //BANKCON5
					      0x00018005,     //BANKCON6
					      0x00018005,     //BANKCON7
					      0x008C07A3,     //REFRESH
					      0x000000B1,     //BANKSIZE
					      0x00000030,     //MRSRB6
					      0x00000030,     //MRSRB7
					    };


    for (; i < 13; i++)
        p[i] = mem_cfg_val[i];
}


/*
 * 设置页表
 */

extern unsigned int MMU_TLB_BASE;
extern unsigned int SYSTEM_TOTAL_MEMORY_START;

 #define VECTOR_BASE SYSTEM_TOTAL_MEMORY_START
//映射1M地址空间
void __set_l1_section_descriptor(unsigned long virtuladdr, unsigned long physicaladdr, unsigned int attributes)
{
    volatile unsigned int *mmu_tlb_base = (unsigned int*)MMU_TLB_BASE;

	*(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | attributes;
}
//映射连续n M地址空间
void set_l1_parallel_descriptor(unsigned long viraddr_start, unsigned long viraddr_end, unsigned long phyaddr_start, unsigned int attributes)
{
	int nSec, i;
	volatile unsigned long *mmu_tlb_base;

	mmu_tlb_base = (unsigned long *)MMU_TLB_BASE + (viraddr_start >> 20);
	nSec = (viraddr_end >> 20) - (viraddr_start >> 20);

	for (i = 0; i < nSec; i++)
		*mmu_tlb_base++ = attributes | (((phyaddr_start >> 20) + i) << 20);
}

void create_page_table(void)
{

    unsigned long virtuladdr, physicaladdr;
    unsigned long *mmu_tlb_base = (unsigned long *)MMU_TLB_BASE;

    /*
     * 将0～1M的虚拟地址映射到0x30000000
     */
    virtuladdr = 0;
    physicaladdr = VECTOR_BASE;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC_WB;
    virtuladdr = 0x51000000;
    physicaladdr = 0x51000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;
    /*
     * 0x56000000是GPIO寄存器的起始物理地址，
     */
    virtuladdr = 0x56000000;
    physicaladdr = 0x56000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;
    
	virtuladdr = 0x48000000;
    physicaladdr = 0x48000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;


    virtuladdr = 0x4a000000;
    physicaladdr = 0x4a000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;

    virtuladdr = 0x4e000000;
    physicaladdr = 0x4e000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;

    virtuladdr = 0x50000000;
    physicaladdr = 0x50000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;
    
	virtuladdr = 0x20000000;
    physicaladdr = 0x20000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC_WB_NCNB;

    /*
     * SDRAM的物理地址范围是0x30000000～0x33FFFFFF，
     * 将虚拟地址0xB0000000～0xB3FFFFFF映射到物理地址0x30000000～0x33FFFFFF上，
     * 总共64M，涉及64个段描述符
     */

	set_l1_parallel_descriptor(0x30000000, 0x34000000, 0x30000000, MMU_SECDESC_WB);
#if 0
    virtuladdr = 0x30000000;
    physicaladdr = 0x30000000;
    while (virtuladdr < 0x34000000)
    {
        *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                                MMU_SECDESC_WB;
        virtuladdr += 0x100000;
        physicaladdr += 0x100000;
    }
#endif
}

/*
 * 启动MMU
 */

void start_mmu(void)
{
	int r0, r4;
    unsigned long ttb = MMU_TLB_BASE;

	__asm
	{
		mov    r0,  0
		mcr    p15, 0, r0, c7, c7,  0   /* 使无效ICaches和DCaches */

		mcr    p15, 0, r0, c7, c10, 4   /* drain write buffer on v4 */
		mcr    p15, 0, r0, c8, c7,  0    /* 使无效指令、数据TLB */

		mov    r4, ttb                  /* r4 = 页表基址 */
		mcr    p15, 0, r4, c2, c0, 0    /* 设置页表基址寄存器 */

		mvn    r0, 0
		mcr    p15, 0, r0, c3, c0, 0    /* 域访问控制寄存器设为0xFFFFFFFF，
					                         * 不进行权限检查*/
		/*
		* 对于控制寄存器，先读出其值，在这基础上修改感兴趣的位，
		* 然后再写入
		*/
		mrc    p15, 0, r0, c1, c0, 0    /* 读出控制寄存器的值 */

		/* 控制寄存器的低16位含义为：.RVI ..RS B... .CAM
		* R : 表示换出Cache中的条目时使用的算法，
		*     0 = Random replacement；1 = Round robin replacement
		* V : 表示异常向量表所在的位置，
		*     0 = Low addresses = 0x00000000；1 = High addresses = 0xFFFF0000
		* I : 0 = 关闭ICaches；1 = 开启ICaches
		* R、S : 用来与页表中的描述符一起确定内存的访问权限
		* B : 0 = CPU为小字节序；1 = CPU为大字节序
		* C : 0 = 关闭DCaches；1 = 开启DCaches
		* A : 0 = 数据访问时不进行地址对齐检查；1 = 数据访问时进行地址对齐检查
		* M : 0 = 关闭MMU；1 = 开启MMU
		*/

		/*
		* 先清除不需要的位，往下若需要则重新设置它们
		*/
		                        /* .RVI ..RS B... .CAM */
		bic    r0, r0, #0x3000          /* ..11 .... .... .... 清除V、I位 */
		bic    r0, r0, #0x0300         /* .... ..11 .... .... 清除R、S位 */
		bic    r0, r0, #0x0087          /* .... .... 1... .111 清除B/C/A/M */

		/*
		* 设置需要的位
		*/
		orr    r0, r0, #0x0002          /* .... .... .... ..1. 开启对齐检查 */
		orr    r0, r0, #0x0004         /* .... .... .... .1.. 开启DCaches */
		orr    r0, r0, #0x1000         /* ...1 .... .... .... 开启ICaches */
		orr    r0, r0, #0x0001         /* .... .... .... ...1 使能MMU */

		mcr    p15, 0, r0, c1, c0, 0    /* 将修改的值写入控制寄存器 */		
	}
}



/*
 * 对于MPLLCON寄存器，[19:12]为MDIV，[9:4]为PDIV，[1:0]为SDIV
 * 有如下计算公式：
 *  S3C2410: MPLL(FCLK) = (m * Fin)/(p * 2^s)
 *  S3C2440: MPLL(FCLK) = (2 * m * Fin)/(p * 2^s)
 *  其中: m = MDIV + 8, p = PDIV + 2, s = SDIV
 * 对于本开发板，Fin = 12MHz
 * 设置CLKDIVN，令分频比为：FCLK:HCLK:PCLK=1:2:4，
 * FCLK=200MHz,HCLK=100MHz,PCLK=50MHz
 */
void init_clock(void)
{
	int r1;
    // LOCKTIME = 0x00ffffff;   // 使用默认值即可
    CLKDIVN  = 0x03;            // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1

    /* 如果HDIVN非0，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode” */
	__asm
	{
		mrc    p15, 0, r1, c1, c0, 0        /* 读出控制寄存器 */
	    orr    r1, r1, #0xc0000000          /* 设置为“asynchronous bus mode” */
	    mcr    p15, 0, r1, c1, c0, 0        /* 写入控制寄存器 */
	}

    /* 判断是S3C2410还是S3C2440 */
    if ((GSTATUS1 == 0x32410000) || (GSTATUS1 == 0x32410002))
    {
        MPLLCON = S3C2410_MPLL_200MHZ;  /* 现在，FCLK=200MHz,HCLK=100MHz,PCLK=50MHz */
    }
    else
    {
        MPLLCON = S3C2440_MPLL_200MHZ;  /* 现在，FCLK=200MHz,HCLK=100MHz,PCLK=50MHz */
    }
}

void timer_init(void)
{
	TCFG0 |= (100 << 8);
    TCFG1 |= (2 << 16);
	TCON &= (~(7 << 20));
	TCON |= (1 << 22);
	TCON |= (1 << 21);

	TCONB4 = 625;
	TCON |= (1 << 20);
	TCON &= ~(1 << 21);

	umask_int(14);
    umask_int(28);
    usubmask_int(0);
	enable_irq();
}




int init_system(void)
{
    disable_watch_dog();
    init_memory();
    create_page_table();
    init_clock();
    start_mmu();
	
	return 0;
}


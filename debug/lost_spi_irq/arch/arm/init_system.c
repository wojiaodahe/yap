#include "s3c24xx.h"
#include "system.h"
#include "config.h"
#include "s3c24xx_irqs.h"

extern void enable_irq(void);
extern void disable_irq(void);

/*
 * �ر�WATCHDOG������CPU�᲻������
 */
void disable_watch_dog(void)
{
    WTCON = 0;
}


void init_memory(void)
{
    int i = 0;
    volatile unsigned long *p = (volatile unsigned long *)MEM_CTL_BASE;

    /* SDRAM 13���Ĵ�����ֵ */
    unsigned long  const    mem_cfg_val[] = {
					      0x22000000,     //BWSCON
					      0x00000700,     //BANKCON0
					      0x00000700,     //BANKCON1
					      0x00000700,     //BANKCON2
					      0x00000700,     //BANKCON3
					      0x00000700,     //BANKCON4
					      0x00000700,     //BANKCON5
					      0x00018001,     //BANKCON6
					      0x00018001,     //BANKCON7
					      0x008404f5,     //REFRESH
					      0x000000B1,     //BANKSIZE
					      0x00000020,     //MRSRB6
					      0x00000020,     //MRSRB7
					    };


    for (; i < 13; i++)
        p[i] = mem_cfg_val[i];
}


/*
 * ����ҳ��
 */

extern unsigned int MMU_TLB_BASE;
extern unsigned int SYSTEM_TOTAL_MEMORY_START;

 #define VECTOR_BASE SYSTEM_TOTAL_MEMORY_START
//ӳ��1M��ַ�ռ�
void __set_l1_section_descriptor(unsigned long virtuladdr, unsigned long physicaladdr, unsigned int attributes)
{
    volatile unsigned int *mmu_tlb_base = (unsigned int*)MMU_TLB_BASE;

	*(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | attributes;
}

//ӳ������n M��ַ�ռ�
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
    volatile unsigned long *mmu_tlb_base = (volatile unsigned long *)MMU_TLB_BASE;

    /*
     * ��0��1M�������ַӳ�䵽0x30000000
     */
    virtuladdr = 0;
    physicaladdr = VECTOR_BASE;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC_WB;
	
    virtuladdr = 0x20000000;
    physicaladdr = 0x20000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC_WB_NCNB;
   

	set_l1_parallel_descriptor(0x30000000, 0x34000000, 0x30000000, MMU_SECDESC_WB);

    virtuladdr = 0x50000000;
    physicaladdr = 0x50000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;
    virtuladdr = 0x51000000;
    physicaladdr = 0x51000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;
    /*
     * 0x56000000��GPIO�Ĵ�������ʼ������ַ��
     */
    virtuladdr = 0x56000000;
    physicaladdr = 0x56000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;
    
    virtuladdr = 0x59000000;
    physicaladdr = 0x59000000;
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

}

/*
 * ����MMU
 */

void start_mmu(void)
{
	int r0, r4;
    unsigned long ttb = MMU_TLB_BASE;

	__asm
	{
		mov    r0,  0
		mcr    p15, 0, r0, c7, c7,  0   /* ʹ��ЧICaches��DCaches */

		mcr    p15, 0, r0, c7, c10, 4   /* drain write buffer on v4 */
		mcr    p15, 0, r0, c8, c7,  0    /* ʹ��Чָ�����TLB */

		mov    r4, ttb                  /* r4 = ҳ����ַ */
		mcr    p15, 0, r4, c2, c0, 0    /* ����ҳ����ַ�Ĵ��� */

		mvn    r0, 0
		mcr    p15, 0, r0, c3, c0, 0    /* ����ʿ��ƼĴ�����Ϊ0xFFFFFFFF��
					                         * ������Ȩ�޼��*/
		/*
		* ���ڿ��ƼĴ������ȶ�����ֵ������������޸ĸ���Ȥ��λ��
		* Ȼ����д��
		*/
		mrc    p15, 0, r0, c1, c0, 0    /* �������ƼĴ�����ֵ */

		/* ���ƼĴ����ĵ�16λ����Ϊ��.RVI ..RS B... .CAM
		* R : ��ʾ����Cache�е���Ŀʱʹ�õ��㷨��
		*     0 = Random replacement��1 = Round robin replacement
		* V : ��ʾ�쳣���������ڵ�λ�ã�
		*     0 = Low addresses = 0x00000000��1 = High addresses = 0xFFFF0000
		* I : 0 = �ر�ICaches��1 = ����ICaches
		* R��S : ������ҳ���е�������һ��ȷ���ڴ�ķ���Ȩ��
		* B : 0 = CPUΪС�ֽ���1 = CPUΪ���ֽ���
		* C : 0 = �ر�DCaches��1 = ����DCaches
		* A : 0 = ���ݷ���ʱ�����е�ַ�����飻1 = ���ݷ���ʱ���е�ַ������
		* M : 0 = �ر�MMU��1 = ����MMU
		*/

		/*
		* ���������Ҫ��λ����������Ҫ��������������
		*/
		                        /* .RVI ..RS B... .CAM */
		bic    r0, r0, #0x3000          /* ..11 .... .... .... ���V��Iλ */
		bic    r0, r0, #0x0300         /* .... ..11 .... .... ���R��Sλ */
		bic    r0, r0, #0x0087          /* .... .... 1... .111 ���B/C/A/M */

		/*
		* ������Ҫ��λ
		*/
		orr    r0, r0, #0x0002          /* .... .... .... ..1. ���������� */
		orr    r0, r0, #0x0004         /* .... .... .... .1.. ����DCaches */
		orr    r0, r0, #0x1000         /* ...1 .... .... .... ����ICaches */
		orr    r0, r0, #0x0001         /* .... .... .... ...1 ʹ��MMU */

		mcr    p15, 0, r0, c1, c0, 0    /* ���޸ĵ�ֵд����ƼĴ��� */		
	}
}


void MMU_SetMTT(int vaddrStart, int vaddrEnd, int paddrStart, int attr)
{
    volatile unsigned int *pTT;
    volatile int i, nSec;

    pTT = (unsigned int *)MMU_TLB_BASE + (vaddrStart >> 20);
    nSec = (vaddrEnd >> 20) - (vaddrStart >> 20);
    for (i = 0; i <= nSec; i++)
        *pTT++ = attr | (((paddrStart >> 20) + i) << 20);
}

void MMU_Init(void)
{
	int i, j;
	//========================== IMPORTANT NOTE =========================
	//The current stack and code area can't be re-mapped in this routine.
	//If you want memory map mapped freely, your own sophiscated MMU
	//initialization code is needed.
	//===================================================================

	MMU_DisableDCache();
	MMU_DisableICache();

	//If write-back is used,the DCache should be cleared.
	for(i=0;i<64;i++)
		for(j=0;j<8;j++)
			MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));
	MMU_InvalidateICache();
    
	#if 0
	//To complete MMU_Init() fast, Icache may be turned on here.
	MMU_EnableICache(); 
	#endif
    
	MMU_DisableMMU();
	MMU_InvalidateTLB();

	//MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr)
	//MMU_SetMTT(0x00000000,0x07f00000,0x00000000,RW_CNB);  //bank0
	MMU_SetMTT(0x00000000,0x03f00000,0x30000000,RW_CB);  //bank0
	MMU_SetMTT(0x04000000,0x07f00000,0,RW_NCNB);  			//bank0
	MMU_SetMTT(0x08000000,0x0ff00000,0x08000000,RW_CNB);  //bank1
	MMU_SetMTT(0x10000000,0x17f00000,0x10000000,RW_NCNB); //bank2
	MMU_SetMTT(0x18000000,0x1ff00000,0x18000000,RW_NCNB); //bank3
	//MMU_SetMTT(0x20000000,0x27f00000,0x20000000,RW_CB); //bank4
	MMU_SetMTT(0x20000000,0x27f00000,0x20000000,RW_NCNB); //bank4 for DM9000
	MMU_SetMTT(0x28000000,0x2ff00000,0x28000000,RW_NCNB); //bank5
	//30f00000->30100000, 31000000->30200000
	MMU_SetMTT(0x30000000,0x33f00000,0x30000000,RW_CB);	  //bank6-1
    
	MMU_SetMTT(0x40000000,0x47f00000,0x40000000,RW_NCNB); //SFR
	MMU_SetMTT(0x48000000,0x5af00000,0x48000000,RW_NCNB); //SFR
	MMU_SetMTT(0x5b000000,0x5b000000,0x5b000000,RW_NCNB); //SFR
	MMU_SetMTT(0x5b100000,0xfff00000,0x5b100000,RW_FAULT);//not used

    
	MMU_SetTTBase(MMU_TLB_BASE);
	MMU_SetDomain(0x55555550|DOMAIN1_ATTR|DOMAIN0_ATTR); 
	//DOMAIN1: no_access, DOMAIN0,2~15=client(AP is checked)
	MMU_SetProcessId(0x0);
	MMU_EnableAlignFault();
    	
	MMU_EnableMMU();
	MMU_EnableICache();
	MMU_EnableDCache(); //DCache should be turned on after MMU is turned on.
}    

/*
 * ����MPLLCON�Ĵ�����[19:12]ΪMDIV��[9:4]ΪPDIV��[1:0]ΪSDIV
 * �����¼��㹫ʽ��
 *  S3C2410: MPLL(FCLK) = (m * Fin)/(p * 2^s)
 *  S3C2440: MPLL(FCLK) = (2 * m * Fin)/(p * 2^s)
 *  ����: m = MDIV + 8, p = PDIV + 2, s = SDIV
 * ���ڱ������壬Fin = 12MHz
 * ����CLKDIVN�����Ƶ��Ϊ��FCLK:HCLK:PCLK=1:2:4��
 * FCLK=200MHz,HCLK=100MHz,PCLK=50MHz
 */
void init_clock(void)
{
	int r1;
    // LOCKTIME = 0x00ffffff;   // ʹ��Ĭ��ֵ����
    CLKDIVN  = 0x05;            // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1

    /* ���HDIVN��0��CPU������ģʽӦ�ôӡ�fast bus mode����Ϊ��asynchronous bus mode�� */
	__asm
	{
		mrc    p15, 0, r1, c1, c0, 0        /* �������ƼĴ��� */
	    orr    r1, r1, #0xc0000000          /* ����Ϊ��asynchronous bus mode�� */
	    mcr    p15, 0, r1, c1, c0, 0        /* д����ƼĴ��� */
	}

    /* �ж���S3C2410����S3C2440 */
    if ((GSTATUS1 == 0x32410000) || (GSTATUS1 == 0x32410002))
    {
        MPLLCON = S3C2410_MPLL_200MHZ;  /* ���ڣ�FCLK=200MHz,HCLK=100MHz,PCLK=50MHz */
    }
    else
    {
        MPLLCON = S3C2440_MPLL_200MHZ;  /* ���ڣ�FCLK=200MHz,HCLK=100MHz,PCLK=50MHz */
    }
}

void s3c24xx_timer4_irq_handler(void *prv)
{
    OS_Clock_Tick();
}

int timer_init(void)
{
	TCFG0 |= (100 << 8);
    TCFG1 |= (2 << 16);
	TCON &= (~(7 << 20));
	TCON |= (1 << 22);
	TCON |= (1 << 21);

	TCONB4 = 625;
	TCON |= (1 << 20);
	TCON &= ~(1 << 21);

    return request_irq(IRQ_TIMER4, s3c24xx_timer4_irq_handler, 0, 0);
}




int init_system(void)
{
    disable_watch_dog();
  
    init_clock();
#if 0
    create_page_table();
    start_mmu();
#else
    MMU_Init();
#endif
	//init_memory();
	return 0;
}

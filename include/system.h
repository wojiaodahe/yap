/*
 *
 * 定义系统初始化时所用的 参数 和 宏
 *
 */
#ifndef __SYSTEM_H__
#define __SYSTEM_H_

/* SDRAM regisers */
#define     MEM_CTL_BASE    0x48000000
#define     SDRAM_BASE      0x30000000


/*
 * mmu 的初始化
 * 用于段描述符的一些宏定义
 */
#define MMU_FULL_ACCESS     (3 << 10)   /* 访问权限 */
#define MMU_DOMAIN          (0 << 5)    /* 属于哪个域 */
#define MMU_SPECIAL         (1 << 4)    /* 必须是1 */
#define MMU_CACHEABLE       (1 << 3)    /* cacheable */
#define MMU_BUFFERABLE      (1 << 2)    /* bufferable */
#define MMU_SECTION         (2)         /* 表示这是段描述符 */
#define MMU_SECDESC         (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | MMU_SECTION)
#define MMU_SECDESC_WB      (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | MMU_CACHEABLE | MMU_BUFFERABLE | MMU_SECTION)
#define MMU_SECDESC_DIS     ((1 << 10) | MMU_DOMAIN | MMU_SPECIAL | MMU_CACHEABLE | MMU_BUFFERABLE | MMU_SECTION)
#define MMU_SECDESC_WB_NCNB (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | MMU_SECTION)
#define MMU_SECTION_SIZE    0x00100000

/*
 * 时钟初始化参数
 */
#define S3C2410_MPLL_200MHZ     ((0x5c << 12) | (0x04 << 4) | (0x00))
#define S3C2440_MPLL_200MHZ     ((92 << 12) | (1 << 4) | (1 << 0))

int SET_IF(void);
void WR_IF(int cpsrValue);
void CLR_IF(void);
void EnterCritical(unsigned int *pSave);
void ExitCritical(unsigned int *pSave);
void MMU_EnableICache(void);
void MMU_DisableICache(void);
void MMU_EnableDCache(void);
void MMU_DisableDCache(void);
void MMU_EnableAlignFault(void);
void MMU_DisableAlignFault(void);
void MMU_EnableMMU(void);
void MMU_DisableMMU(void);
void MMU_SetTTBase(unsigned int base);
void MMU_SetDomain(unsigned int domain);

void MMU_SetFastBusMode(void);  //GCLK=HCLK
void MMU_SetAsyncBusMode(void); //GCLK=FCLK @(FCLK>=HCLK)

void MMU_InvalidateIDCache(void);
void MMU_InvalidateICache(void);
void MMU_InvalidateICacheMVA(unsigned int mva);
void MMU_PrefetchICacheMVA(unsigned int mva);
void MMU_InvalidateDCache(void);
void MMU_InvalidateDCacheMVA(unsigned int mva);
void MMU_CleanDCacheMVA(unsigned int mva);
void MMU_CleanInvalidateDCacheMVA(unsigned int mva);
void MMU_CleanDCacheIndex(unsigned int index);
void MMU_CleanInvalidateDCacheIndex(unsigned int index);	
void MMU_WaitForInterrupt(void);
	
void MMU_InvalidateTLB(void);
void MMU_InvalidateITLB(void);
void MMU_InvalidateITLBMVA(unsigned int mva);
void MMU_InvalidateDTLB(void);
void MMU_InvalidateDTLBMVA(unsigned int mva);

void MMU_SetDCacheLockdownBase(unsigned int base);
void MMU_SetICacheLockdownBase(unsigned int base);

void MMU_SetDTLBLockdown(unsigned int baseVictim);
void MMU_SetITLBLockdown(unsigned int baseVictim);

void MMU_SetProcessId(unsigned int pid);

#define DESC_SEC	(0x2|(1<<4))
#define CB		(3<<2)  //cache_on, write_back
#define CNB		(2<<2)  //cache_on, write_through 
#define NCB             (1<<2)  //cache_off,WR_BUF on
#define NCNB		(0<<2)  //cache_off,WR_BUF off
#define AP_RW		(3<<10) //supervisor=RW, user=RW
#define AP_RO		(2<<10) //supervisor=RW, user=RO

#define DOMAIN_FAULT	(0x0)
#define DOMAIN_CHK	(0x1) 
#define DOMAIN_NOTCHK	(0x3) 
#define DOMAIN0		(0x0<<5)
#define DOMAIN1		(0x1<<5)

#define DOMAIN0_ATTR	(DOMAIN_CHK<<0) 
#define DOMAIN1_ATTR	(DOMAIN_FAULT<<2) 

#define RW_CB		(AP_RW|DOMAIN0|CB|DESC_SEC)
#define RW_CNB		(AP_RW|DOMAIN0|CNB|DESC_SEC)
#define RW_NCNB		(AP_RW|DOMAIN0|NCNB|DESC_SEC)
#define RW_FAULT	(AP_RW|DOMAIN1|NCNB|DESC_SEC)

void MMU_Init(void);
void MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr);
void ChangeRomCacheStatus(int attr);


#endif

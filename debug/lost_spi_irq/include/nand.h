#ifndef __NAND_H__
#define __NAND_H__

#define NUM_BLOCKS			0x1000	//  64 MB Smartmedia card.
#define SECTOR_SIZE			512
#define SPARE_SIZE			16
#define PAGES_PER_BLOCK			32

//  For flash chip that is bigger than 32 MB, we need to have 4 step address
//  
#define NFCONF_INIT			0xF830  // 512-byte 4 Step Address
#define NEED_EXT_ADDR			1
//#define NFCONF_INIT			0xA830  // 256-byte 4 Step Address
//#define NEED_EXT_ADDR			0

//#define NFCONF_INIT			0xF840

//  NAND Flash Command. only for K9F1208UOM

#define CMD_READ1			0x00	//  Read
//#define CMD_READ1			0x01	//  Read1
//#define CMD_READ2			0x50	//  Read2
#define CMD_READ2			0x30	//  Read3---这里是针对K9F2G08UOB而言的，页内没有分为两部分，所以用这个指令即可；
#define CMD_READID			0x90	//  ReadID
#define CMD_WRITE1			0x80	//  Write phase 1
#define CMD_WRITE2			0x10	//  Write phase 2
#define CMD_ERASE1			0x60	//  Erase phase 1
#define CMD_ERASE2			0xd0	//  Erase phase 2
#define CMD_STATUS			0x70	//  Status read
#define CMD_RESET			0xff	//  Reset
#define CMD_RANDOMREAD1		0x05 	//随意读命令周期1
#define CMD_RANDOMREAD2 	0xE0 	//随意读命令周期2
#define CMD_RANDOMWRITE 	0x85 	//随意写命令

//  Status bit pattern
#define STATUS_READY			0x40	//  Ready
#define STATUS_ERROR			0x01	//  Error

//  Status bit pattern
#define STATUS_READY			0x40
#define STATUS_ERROR			0x01

#define NF_Send_Cmd(cmd)	{NFCMD  = (cmd); }
#define NF_Send_Addr(addr)	{NFADDR = (addr); }
#define NF_Send_Data(data)	{NFDATA8 = (data); }	
#define NF_Enable()			{NFCONT &= ~(1<<1); }           //nand flash控制器使能
#define NF_Disable()		{NFCONT |= (1<<1); }
#define NF_Enable_RB()		{NFSTAT |= (1<<2); }         //开启RnB监视模式；
#define NF_Check_Busy()		{while(!(NFSTAT&(1<<2)));}  //相当于等待RnB置1----这说明nand flash不忙了
#define NF_Read_Byte()		(NFDATA8)


#define NF_RSTECC()			{NFCONT |= (1<<4); }
#define NF_RDMECC()			(NFMECC0 )
#define NF_RDSECC()			(NFSECC )
#define NF_RDDATA()			(NFDATA)

#define NF_WAITRB()			{while(!(NFSTAT&(1<<0)));} 
#define NF_MECC_UnLock()	{NFCONT &= ~(1<<5); }
#define NF_MECC_Lock()		{NFCONT |= (1<<5); }
#define NF_SECC_UnLock()	{NFCONT &= ~(1<<6); }
#define NF_SECC_Lock()		{NFCONT |= (1<<6); }

#define	RdNFDat8()			(NFDATA8)	//byte access
#define	RdNFDat()			RdNFDat8()	//for 8 bit nand flash, use byte access
#define	WrNFDat8(dat)		(NFDATA8 = (dat))	//byte access
#define	WrNFDat(dat)		WrNFDat8()	//for 8 bit nand flash, use byte access

#define pNFCONF				NFCONF 
#define pNFCMD				NFCMD  
#define pNFADDR				NFADDR 
#define pNFDATA				NFDATA 
#define pNFSTAT				NFSTAT 
#define pNFECC				NFECC0  

//#define NF_CE_L()			NF_nFCE_L()
//#define NF_CE_H()			NF_nFCE_H()
#define NF_DATA_R()			NFDATA
#define NF_ECC()			NFECC0

// HCLK=100Mhz
#define TACLS				1	// 1-clk(0ns) 
#define TWRPH0				4	// 3-clk(25ns)
#define TWRPH1				0	// 1-clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns




extern char rNF_ReadID(void) ;
extern void NF_Init(void) ;
extern void NF_ReadPage(unsigned int block,unsigned int page, char *dstaddr) ;
extern void LB_ReadPage(unsigned int addr, char *dstaddr) ;//这段程序用于，Nand Flash每页大小是2048个字节
extern void NF_WritePage(unsigned int block,unsigned int page, char *buffer) ;
extern int  NF_EraseBlock(unsigned int block) ;//
extern unsigned char NF_RamdomRead(unsigned int block,unsigned int page,unsigned int add);
extern unsigned char NF_RamdomWrite(unsigned int block,unsigned int page,unsigned int add,unsigned char dat) ;

#endif /*__NAND_H__*/

#ifndef __S3C24XX_H__
#define __S3C24XX_H__

/* WOTCH DOG register */
#define     WTCON           (*(volatile unsigned long *)0x53000000)


//Nand Flash
#define NFCONF		(*(volatile unsigned long *)0x4E000000)		//NAND Flash configuration
#define NFCONT		(*(volatile unsigned long *)0x4E000004)     //NAND Flash control
#define NFCMD		(*(volatile unsigned long *)0x4E000008)     //NAND Flash command
#define NFADDR		(*(volatile unsigned long *)0x4E00000C)     //NAND Flash address
#define NFDATA		(*(volatile unsigned long *)0x4E000010)     //NAND Flash data
#define NFDATA8		(*(volatile unsigned char *)0x4E000010) 	//NAND Flash data   							//NAND Flash data address
#define NFMECCD0	(*(volatile unsigned long *)0x4E000014)     //NAND Flash ECC for Main Area
#define NFMECCD1	(*(volatile unsigned long *)0x4E000018)
#define NFSECCD		(*(volatile unsigned long *)0x4E00001C)		//NAND Flash ECC for Spare Area
#define NFSTAT		(*(volatile unsigned long *)0x4E000020)		//NAND Flash operation status
#define NFESTAT0	(*(volatile unsigned long *)0x4E000024)
#define NFESTAT1	(*(volatile unsigned long *)0x4E000028)
#define NFMECC0		(*(volatile unsigned long *)0x4E00002C)
#define NFMECC1		(*(volatile unsigned long *)0x4E000030)
#define NFSECC		(*(volatile unsigned long *)0x4E000034)
#define NFSBLK		(*(volatile unsigned long *)0x4E000038)		//NAND Flash Start block address
#define NFEBLK		(*(volatile unsigned long *)0x4E00003C)		//NAND Flash End block address


/*GPIO registers*/
#define GPACON    			(*(volatile unsigned long *)0x56000000)	//Port A control
#define GPADAT    			(*(volatile unsigned long *)0x56000004)	//Port A data

#define GPBCON              (*(volatile unsigned long *)0x56000010)
#define GPBDAT              (*(volatile unsigned long *)0x56000014)

#define GPFCON              (*(volatile unsigned long *)0x56000050)
#define GPFDAT              (*(volatile unsigned long *)0x56000054)
#define GPFUP               (*(volatile unsigned long *)0x56000058)

#define GPGCON              (*(volatile unsigned long *)0x56000060)
#define GPGDAT              (*(volatile unsigned long *)0x56000064)
#define GPGUP               (*(volatile unsigned long *)0x56000068)

#define GPHCON              (*(volatile unsigned long *)0x56000070)
#define GPHDAT              (*(volatile unsigned long *)0x56000074)
#define GPHUP               (*(volatile unsigned long *)0x56000078)



/*UART registers*/
#define ULCON0              (*(volatile unsigned long *)0x50000000)
#define UCON0               (*(volatile unsigned long *)0x50000004)
#define UFCON0              (*(volatile unsigned long *)0x50000008)
#define UMCON0              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)
#define UTXH0               (*(volatile unsigned char *)0x50000020)
#define URXH0               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)


/*interrupt registes*/
#define SRCPND              (*(volatile unsigned long *)0x4A000000)
#define INTMOD              (*(volatile unsigned long *)0x4A000004)
#define INTMSK              (*(volatile unsigned long *)0x4A000008)
#define PRIORITY            (*(volatile unsigned long *)0x4A00000c)
#define INTPND              (*(volatile unsigned long *)0x4A000010)
#define INTOFFSET           (*(volatile unsigned long *)0x4A000014)
#define SUBSRCPND           (*(volatile unsigned long *)0x4A000018)
#define INTSUBMSK           (*(volatile unsigned long *)0x4A00001c)

/*external interrupt registers*/
#define EXTINT0             (*(volatile unsigned long *)0x56000088)
#define EXTINT1             (*(volatile unsigned long *)0x5600008c)
#define EXTINT2             (*(volatile unsigned long *)0x56000090)
#define EINTMASK            (*(volatile unsigned long *)0x560000a4)
#define EINTPEND            (*(volatile unsigned long *)0x560000a8)

#define BWSCON			    (*(volatile unsigned long *)0x48000000)
#define BANKCON4			(*(volatile unsigned long *)0x48000014)

/*clock registers*/
#define	LOCKTIME		(*(volatile unsigned long *)0x4c000000)
#define	MPLLCON		(*(volatile unsigned long *)0x4c000004)
#define	UPLLCON		(*(volatile unsigned long *)0x4c000008)
#define	CLKCON		(*(volatile unsigned long *)0x4c00000c)
#define	CLKSLOW		(*(volatile unsigned long *)0x4c000010)
#define	CLKDIVN		(*(volatile unsigned long *)0x4c000014)


/*PWM & Timer registers*/
#define	TCFG0		(*(volatile unsigned long *)0x51000000)
#define	TCFG1		(*(volatile unsigned long *)0x51000004)
#define	TCON		(*(volatile unsigned long *)0x51000008)
#define	TCNTB0		(*(volatile unsigned long *)0x5100000c)
#define	TCMPB0		(*(volatile unsigned long *)0x51000010)
#define	TCNTO0		(*(volatile unsigned long *)0x51000014)
#define TCONB4  	(*(volatile unsigned long *)(0x51000000 + 0x3c))

/* SPI */
#define	SPCON0		(*(volatile unsigned long *)0x59000000)
#define	SPSTA0		(*(volatile unsigned long *)0x59000004)
#define	SPPIN0		(*(volatile unsigned long *)0x59000008)
#define	SPPRE0		(*(volatile unsigned long *)0x5900000C)
#define	SPTDAT0	(*(volatile unsigned char *)0x59000010)
#define	SPRDAT0	(*(volatile unsigned char *)0x59000014)

#define	SPCON1		(*(volatile unsigned long *)0x59000020)
#define	SPSTA1		(*(volatile unsigned long *)0x59000024)
#define	SPPIN1		(*(volatile unsigned long *)0x59000028)
#define	SPPRE1		(*(volatile unsigned long *)0x5900002C)
#define	SPTDAT1	(*(volatile unsigned char *)0x59000030)
#define	SPRDAT1 	(*(volatile unsigned char *)0x59000034)

/* I2C registers */
#define IICCON  	(*(volatile unsigned *)0x54000000) // IIC control
#define IICSTAT 	(*(volatile unsigned *)0x54000004) // IIC status
#define IICADD  	(*(volatile unsigned *)0x54000008) // IIC address
#define IICDS   	(*(volatile unsigned *)0x5400000c) // IIC data shift
#define IICLC		(*(volatile unsigned *)0x54000010)	 //IIC multi-master line control


// LCD CONTROLLER
#define LCDCON1     (*(volatile unsigned long *)0x4d000000) //LCD control 1
#define LCDCON2     (*(volatile unsigned long *)0x4d000004) //LCD control 2
#define LCDCON3     (*(volatile unsigned long *)0x4d000008) //LCD control 3
#define LCDCON4     (*(volatile unsigned long *)0x4d00000c) //LCD control 4
#define LCDCON5     (*(volatile unsigned long *)0x4d000010) //LCD control 5
#define LCDSADDR1   (*(volatile unsigned long *)0x4d000014) //STN/TFT Frame buffer start address 1
#define LCDSADDR2   (*(volatile unsigned long *)0x4d000018) //STN/TFT Frame buffer start address 2
#define LCDSADDR3   (*(volatile unsigned long *)0x4d00001c) //STN/TFT Virtual screen address set
#define REDLUT      (*(volatile unsigned long *)0x4d000020) //STN Red lookup table
#define GREENLUT    (*(volatile unsigned long *)0x4d000024) //STN Green lookup table 
#define BLUELUT     (*(volatile unsigned long *)0x4d000028) //STN Blue lookup table
#define DITHMODE    (*(volatile unsigned long *)0x4d00004c) //STN Dithering mode
#define TPAL        (*(volatile unsigned long *)0x4d000050) //TFT Temporary palette
#define LCDINTPND   (*(volatile unsigned long *)0x4d000054) //LCD Interrupt pending
#define LCDSRCPND   (*(volatile unsigned long *)0x4d000058) //LCD Interrupt source
#define LCDINTMSK   (*(volatile unsigned long *)0x4d00005c) //LCD Interrupt mask
#define LPCSEL      (*(volatile unsigned long *)0x4d000060) //LPC3600 Control

#define GSTATUS1    (*(volatile unsigned long *)0x560000B0)

struct s3c24xx_gpio
{
	volatile unsigned long gpxcon;
	volatile unsigned long gpxdata;
	volatile unsigned long gpxup;
};

#endif


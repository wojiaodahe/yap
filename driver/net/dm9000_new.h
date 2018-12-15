#ifndef __DM9000_H 
#define __DM9000_H 

//#define DM9000_ID		0x90000A46

#define DM9000_NCR             0x00
#define DM9000_NSR             0x01
#define DM9000_TCR             0x02
#define DM9000_TSR1            0x03
#define DM9000_TSR2            0x04
#define DM9000_RCR             0x05
#define DM9000_RSR             0x06
#define DM9000_ROCR            0x07
#define DM9000_BPTR            0x08
#define DM9000_FCTR            0x09
#define DM9000_FCR             0x0A
#define DM9000_EPCR            0x0B
#define DM9000_EPAR            0x0C
#define DM9000_EPDRL           0x0D
#define DM9000_EPDRH           0x0E
#define DM9000_WCR             0x0F

#define DM9000_PAR             0x10
#define DM9000_MAR             0x16

#define DM9000_GPCR			   0x1e
#define DM9000_GPR			   0x1f
#define DM9000_TRPAL           0x22
#define DM9000_TRPAH           0x23
#define DM9000_RWPAL           0x24
#define DM9000_RWPAH           0x25

#define DM9000_VIDL            0x28
#define DM9000_VIDH            0x29
#define DM9000_PIDL            0x2A
#define DM9000_PIDH            0x2B

#define DM9000_CHIPR           0x2C
#define DM9000_SMCR            0x2F

#define DM9000_MRCMDX          0xF0
#define DM9000_MRCMD           0xF2
#define DM9000_MRRL            0xF4
#define DM9000_MRRH            0xF5
#define DM9000_MWCMDX          0xF6
#define DM9000_MWCMD           0xF8
#define DM9000_MWRL            0xFA
#define DM9000_MWRH            0xFB
#define DM9000_TXPLL           0xFC
#define DM9000_TXPLH           0xFD
#define DM9000_ISR             0xFE
#define DM9000_IMR             0xFF

/*PHY registers*/
#define DM9000_BMCR				0x00
#define DM9000_BMSR				0x01
#define DM9000_PHYID1			0x02
#define DM9000_PHYID2			0x03
#define DM9000_ANAR				0x04
#define DM9000_ANLPAR			0x05
#define DM9000_ANER				0x06
#define DM9000_DSCR				0x16
#define DM9000_DSCSR			0x17
#define DM9000_10BTCSR			0x18

typedef struct dm9000_info
{
    void *io_addr;
    void *io_data;

    unsigned short tx_pkt_cnt;
    unsigned short queue_pkt_len;
    unsigned char  phy_addr;
    unsigned char  imr_all;

    unsigned int   flags;   
    
    struct net_device *ndev;
}dm9000_info_t;

#endif /* _DM9000X_H_ */



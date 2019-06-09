/*
 * dm9000 Ethernet
 */ 
#ifndef __DM9000_H 
#define __DM9000_H 

//#define DM9000_ID		0x90000A46

/* although the registers are 16 bit, they are 32-bit aligned.
 */

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

#define Printf printk

#define DM9000_PKT_READY        0x01

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef struct eth_hdr 			//以太网头部结构，为了以后使用方便
{
	U8	d_mac[6];   		//目的地址
	U8	s_mac[6];   		//源地址
	U16	type;    			//协议类型
}ETH_HDR;
typedef struct _arp_hdr 		//以太网头部+ARP首部结构
{
	ETH_HDR ethhdr;				//以太网首部
	U16	hwtype;     		//硬件类型(1表示传输的是以太网MAC地址)
	U16	protocol;			//协议类型(0x0800表示传输的是IP地址)
	U8	hwlen;				//硬件地址长度(6)
	U8	protolen;			//协议地址长度(4)
	U16	opcode; 			//操作(1表示ARP请求,2表示ARP应答)
	U8	smac[6];			//发送端MAC地址
	U8	sipaddr[4];			//发送端IP地址
	U8	dmac[6];			//目的端MAC地址
	U8	dipaddr[4];			//目的端IP地址
}ARP_HDR;

typedef struct ip_hdr			//以太网头部+IP首部结构
{
	ETH_HDR ethhdr;    			//以太网首部
	U8	vhl;      			//4位版本号4位首部长度(0x45)
	U8	tos;				//服务类型(0)
	U16	len;				//整个IP数据报总字节长度
	U16	ipid;           	//IP标识
	U16	ipoffset;     		//3位标识13位偏移
	U8 	ttl;             	//生存时间(32或64)
	U8	proto;         		//协议(1表示ICMP,2表示IGMP,6表示TCP,17表示UDP)
	U16 	ipchksum;    		//首部校验和
	U8 	srcipaddr[4];    	//源IP
	U8	destipaddr[4];   	//目的IP
}IP_HDR;


//void DM9000_init(struct netif *);
void PrintfDM9000Reg(void);
void Print_HostMAC(void);
void TestDm9000(void);
U32 receivepacket(void *priv);
void arp_request(void);
void testNetwork(void); 
#endif /* _DM9000X_H_ */

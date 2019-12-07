#ifndef INCLUDE_ARP_H_
#define INCLUDE_ARP_H_

#include "socket.h"
#include "netdevice.h"

struct arp_hdr
{
	unsigned short hwtype;     		//硬件类型(1表示传输的是以太网MAC地址)
	unsigned short protocol;		//协议类型(0x0800表示传输的是IP地址)
	unsigned char  hwlen;			//硬件地址长度(6)
	unsigned char  protolen;		//协议地址长度(4)
	unsigned short type; 			//操作(1表示ARP请求,2表示ARP应答)
	unsigned char  smac[6];			//发送端MAC地址
	unsigned int   saddr;  			//源ip
	unsigned char  dmac[6];			//目的端MAC地址
	unsigned int   daddr;			//目的ip
}__attribute__((packed));

#define ARP_REQ		1
#define ARP_RSP		2

#define ARP_HW_TYPE_ETH	1


#define MAX_ARP_TABLE_NUM	256
struct arp_table
{
	unsigned char mac[6];
	unsigned int ip;
	unsigned int type;
	struct net_device *ndev;
};

extern int arp_process(struct sk_buff *skb);
extern int arp_init(void);
extern int add_arp_table(unsigned char *mac, unsigned int ip, struct net_device *ndev);
extern void delete_arp_table(void);
extern struct arp_table *search_arp_table(unsigned int ip);
extern void add_skb_to_arp_send_q(struct sk_buff *skb);
extern int arp_send_request(unsigned int ip, struct net_device *ndev);
extern  void updata_arp_table(struct sk_buff *skb);

#endif /* INCLUDE_ARP_H_ */


#ifndef INCLUDE_ARP_H_
#define INCLUDE_ARP_H_

#include "socket.h"
#include "netdevice.h"

struct arp_hdr
{
	unsigned short hwtype;     		//Ӳ������(1��ʾ���������̫��MAC��ַ)
	unsigned short protocol;		//Э������(0x0800��ʾ�������IP��ַ)
	unsigned char  hwlen;			//Ӳ����ַ����(6)
	unsigned char  protolen;		//Э���ַ����(4)
	unsigned short type; 			//����(1��ʾARP����,2��ʾARPӦ��)
	unsigned char  smac[6];			//���Ͷ�MAC��ַ
	unsigned int   saddr;  			//Դip
	unsigned char  dmac[6];			//Ŀ�Ķ�MAC��ַ
	unsigned int   daddr;			//Ŀ��ip
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

#endif /* INCLUDE_ARP_H_ */


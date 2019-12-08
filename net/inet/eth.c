#include "ip.h"
#include "arp.h"
#include "eth.h"
#include "netdevice.h"
#include "socket.h"
#include "inet.h"
#include "printk.h"
#include "lib.h"

void print_eth_head(struct sk_buff *skb)
{
	int i;
	struct ethhdr *eth;

	eth = (struct ethhdr *)(skb->data_buf);

	printk("data len: %d\n", skb->data_len);

	printk("dmac: ");
	for (i = 0; i < 6; i++)
		printk("%x ", eth->h_dest[i]);
	printk("\n");

	printk("smac: ");
	for (i = 0; i < 6; i++)
		printk("%x ", eth->h_source[i]);
	printk("\n");

	printk("proto: %x\n", ntohs(eth->h_proto));

}

int eth_recv(struct sk_buff *skb)
{
    int ret = 0;
	struct ethhdr *eth;

	eth = (struct ethhdr *)skb->data_buf;

	switch (ntohs(eth->h_proto))
	{
	case ETH_P_IP:
        updata_arp_table(skb);
		ret = ip_recv(skb);
        break;
	case ETH_P_ARP:
		ret = arp_process(skb);
        break;
	default:
		free_skbuff(skb);
		break;
	}
	return ret;
}

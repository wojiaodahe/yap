#include "ip.h"
#include "arp.h"
#include "eth.h"
#include "netdevice.h"
#include "socket.h"
#include "inet.h"
#include "printk.h"
#include "syslib.h"

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

int netif_rx(struct sk_buff *skb)
{
	struct ethhdr *eth;

	eth = (struct ethhdr *)skb->data_buf;

	switch (ntohs(eth->h_proto))
	{
	case ETH_P_IP:
		return ip_recv(skb);
	case ETH_P_ARP:
		return arp_process(skb);
	default:
		free_skbuff(skb);
		break;
	}
	return 0;
}

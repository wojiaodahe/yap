/*
 * ip.c
 *
 *  Created on: 2018Äê6ÔÂ27ÈÕ
 *      Author: crane
 */

#include "ip.h"
#include "arp.h"
#include "eth.h"
#include "icmp.h"
#include "error.h"
#include "socket.h"
#include "common.h"
#include "inet_socket.h"
#include "netdevice.h"

static unsigned short ip_data_id = 1000;

#define IP_FRAG_LIST_HEAD_NUM 32
struct ipfrag
{
	unsigned short id;
	struct sk_buff *skb;
	struct list_head head;
	unsigned short current_len;
	unsigned short original_len;
};

static struct ipfrag ip_frag_list_head[IP_FRAG_LIST_HEAD_NUM];

static struct ipfrag *search_frag_list(unsigned short id)
{
	int i;
	for (i = 0; i < IP_FRAG_LIST_HEAD_NUM; i++)
	{
		if (ip_frag_list_head[i].id == id)
			return &ip_frag_list_head[i];
	}

	return NULL;
}

extern struct net_device *return_ndev();
struct net_device *ip_route(struct ip_addr *dest)
{
	return return_ndev();
}


int ip_do_send(struct sk_buff *skb, struct ip_addr *dest, unsigned char proto, struct net_device *ndev)
{
	struct ethhdr    *eth;
	struct iphdr     *iph;
	struct arp_table *arpt = NULL;
	int ret;

	eth = (struct ethhdr *)(skb->data_buf);

	skb->ndev = ndev;
	arpt = search_arp_table(dest->addr);
	if (!arpt)
	{
		arp_send_request(dest->addr, ndev);
		add_skb_to_arp_send_q(skb);
		return 0;
	}

	//copy smac
	memcpy(eth->h_dest, arpt->mac, 6);
	memcpy(eth->h_source, skb->ndev->macaddr, 6);
	eth->h_proto = htons(ETH_P_IP);

	skb->ndev->hard_start_xmit(skb, skb->ndev);

	return 0;
}

int ip_fragment_create()
{
	return 0;
}

int ip_try_fragment_glue()
{
	return 0;
}

int ip_do_recv()
{
	return 0;
}

int ip_recv(struct sk_buff *skb)
{
	struct iphdr *iph;

	iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);
	if (iph->frag_off)
		return ip_try_fragment_glue(skb);

	return ip_do_recv(skb);
}


int ip_fragment(struct sk_buff *skb, struct ip_addr *destaddr, unsigned char proto, struct net_device *ndev)
{
	unsigned short ip_data_len = 0;
	unsigned short total_len = 0;
	unsigned short fragoff = 0;
	struct iphdr *iph;
	struct iphdr *iphnew;
	struct sk_buff *skbnew;
	char *src;
	char *dest;
	int ret;

	ip_data_id++;

	iph  = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);
	iph->tot_len = htons(skb->data_len - SIZEOF_ETHHDR);
	total_len = skb->data_len - SIZEOF_ETHHDR - SIZEOF_IPHDR;
	while (total_len)
	{
		skbnew = alloc_skbuff(ndev->mtu);
		if (!skbnew)
		{
			free_skbuff(skb);
			return -ENOMEM;
		}

		iphnew = (struct iphdr *)(skbnew->data_buf + OFFSET_IPHDR);
		src  = skb->data_buf + OFFSET_UDPHDR + fragoff;
		dest = skbnew->data_buf + OFFSET_UDPHDR;

		ip_data_len = (ndev->mtu - SIZEOF_ETHHDR - SIZEOF_IPHDR) / 8;
		ip_data_len *= 8;
		if (ip_data_len > total_len)
			ip_data_len = total_len;

		memcpy(dest, src, ip_data_len);
		total_len -= ip_data_len;
		
		if (total_len != 0)
			iphnew->frag_off = (htons((fragoff >> 3) | IP_MF));
		else
			iphnew->frag_off = (htons(fragoff >> 3));

		fragoff += ip_data_len;

		iphnew->daddr    = destaddr->addr;
		iphnew->saddr    = htonl(ndev->ip);
		iphnew->protocol = proto;
		iphnew->ttl 	 = 128;
		iphnew->tos	     = 0;
		iphnew->id 		 = htons(ip_data_id);
		iphnew->version  = ((4 << 4) | 5);
		iphnew->tot_len  = htons(ip_data_len + SIZEOF_IPHDR);
		iphnew->check    = 0;
		iphnew->check    = inet_chksum(iphnew, SIZEOF_IPHDR);

		skbnew->data_len = ip_data_len + SIZEOF_IPHDR + SIZEOF_ETHHDR;
		ret = ip_do_send(skbnew, destaddr, proto, ndev);
		if (ret < 0)
		{
			free_skbuff(skb);
			return -EIO;
		}
	}

	free_skbuff(skb);
	return 0;
}

int ip_send(struct sk_buff *skb, struct ip_addr *dest, unsigned char proto)
{
	struct net_device *ndev;
	struct iphdr *iph;

	ndev = ip_route(dest);
	if (!ndev)
		return -ENETUNREACH;/* dest is unreachable */

	skb->ndev = ndev;
	if (skb->data_len > ndev->mtu)
		return ip_fragment(skb, dest, proto, ndev);

	iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);

	ip_data_id++;
	iph->daddr    = dest->addr;
	iph->saddr    = htonl(ndev->ip);
	iph->protocol = proto;
	iph->ttl 	  = 128;
	iph->tos	  = 0;
	iph->id 	  = htons(ip_data_id);
	iph->version  = ((4 << 4) | 5);
	iph->tot_len  = htons(skb->data_len - SIZEOF_ETHHDR);
	iph->check    = 0;
	iph->check    = inet_chksum(iph, SIZEOF_IPHDR);
	
	return ip_do_send(skb, dest, proto, ndev);
}

int frag_test(void)
{
	int i;
	struct sk_buff *skb;
	char *data;
	struct ip_addr dest;
	struct iphdr *iph;

	skb = alloc_skbuff(2000);
	data = skb->data_buf + OFFSET_UDPHDR;
	for (i = 0; i < 2000 - SIZEOF_IPHDR - SIZEOF_ETHHDR; i++)
		data[i] = i & 0xff;

	skb->data_len = 2000;
	dest.addr = htonl(0xc0a80107);

	iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);
	iph->tot_len = htons(2000 - SIZEOF_ETHHDR);
	return ip_send(skb, &dest, 0x11);
}

void print_ip(struct iphdr *iph)
{
	printk("version: %d\n", (iph->version >> 4) & 0xf);
	printk("hdr len: %d\n", (iph->version) & 0xf);
	printk("tos: %d\n", iph->tos);
	printk("tot_len: %d\n", ntohs(iph->tot_len));
	printk("id: %d\n", ntohs(iph->id));
	printk("ttl: %d\n", iph->ttl);
	printk("frag_off: %d\n", iph->frag_off);
	printk("protocol: %x\n", iph->protocol);
	printk("saddr: %x\n", ntohl(iph->saddr));
	printk("daddr: %x\n", ntohl(iph->daddr));
}

int ip_process(struct sk_buff *skb)
{
	struct iphdr *iph;

	iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);
	print_ip(iph);
	switch (iph->protocol)
	{
	case PROTO_ICMP:
		return icmp_process(skb);
		break;
	case PROTO_UDP:
		return udp_process(skb);
		break;
	case PROTO_TCP:
		break;
	default:
		free_skbuff(skb);
	}

	return 0;
}

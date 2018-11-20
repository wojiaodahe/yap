/*
 * icmp.c
 *
 *  Created on: 2018年6月27日
 *      Author: crane
 */

#include "ip.h"
#include "eth.h"
#include "icmp.h"
#include "socket.h"
#include "common.h"
#include "inet_socket.h"
#include "netdevice.h"

void print_icmp(struct icmphdr *icmph)
{
	printk("type: %d\n", icmph->type);
	printk("code: %d\n", icmph->code);
	printk("chksum: %d\n", ntohs(icmph->chksum));
	printk("seqno: %d\n", ntohs(icmph->seqno));
}

/******************************************************************************
 *skb     : sk_buff
 *dest    : 目的 ip
 *type    : icmp 报文 类型(request or reply or ...)
 *code	  :
 *data_len: icmp报文数据长度(不包括icmp头)
 ******************************************************************************/
int icmp_send(struct sk_buff *skb, struct ip_addr *dest, unsigned char type, unsigned char code, unsigned short data_len)
{
	struct icmphdr *icmph;
	icmph = (struct icmphdr *)(skb->data_buf + OFFSET_ICMPHDR);

	icmph->code = code;
	icmph->type = type;
	icmph->chksum = 0;
	icmph->chksum = inet_chksum(icmph, SIZEOF_ICMPHDR + data_len);
	return ip_send(skb, dest, PROTO_ICMP);
}

int icmp_process(struct sk_buff *skb)
{
	struct icmphdr *icmph;
	struct iphdr   *iph;
	struct ip_addr dest;

	iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);
	dest.addr = iph->saddr;

	skb->data_len = ntohs(iph->tot_len) + SIZEOF_ETHHDR;
	return icmp_send(skb, &dest, ICMP_ER, 0, ntohs(iph->tot_len) - SIZEOF_IPHDR - SIZEOF_ICMPHDR);
}
















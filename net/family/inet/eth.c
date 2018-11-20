/*
 * eth.c
 *
 *  Created on: 2018Äê6ÔÂ27ÈÕ
 *      Author: crane
 */

#include "ip.h"
#include "arp.h"
#include "eth.h"
#include "netdevice.h"
#include "socket.h"
//#include "dhcp.h"

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

#if 0

unsigned short inet_chksum(void *dataptr, unsigned short len)
{
  unsigned int acc;
  unsigned short src;
  unsigned char *octetptr;

  acc = 0;
  /* dataptr may be at odd or even addresses */
  octetptr = (unsigned char*)dataptr;
  while (len > 1) {
    /* declare first octet as most significant
       thus assume network order, ignoring host order */
    src = (*octetptr) << 8;
    octetptr++;
    /* declare second octet as least significant */
    src |= (*octetptr);
    octetptr++;
    acc += src;
    len -= 2;
  }
  if (len > 0) {
    /* accumulate remaining octet */
    src = (*octetptr) << 8;
    acc += src;
  }
  /* add deferred carry bits */
  acc = (acc >> 16) + (acc & 0x0000ffffUL);
  if ((acc & 0xffff0000UL) != 0) {
    acc = (acc >> 16) + (acc & 0x0000ffffUL);
  }
  /* This maybe a little confusing: reorder sum using htons()
     instead of ntohs() since it has a little less call overhead.
     The caller must invert bits for Internet sum ! */
  return htons((unsigned short)acc);
}
#else
unsigned short inet_chksum(char *buffer, unsigned short size)
{
	__packed unsigned short *tmp;
	unsigned long cksum=0;

	tmp = buffer;
	while(size >1)
	{
		cksum += *tmp++;
		size -= sizeof(unsigned short);
	}
	if(size) 
		cksum += *tmp;
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (unsigned short)(~cksum);
}
#endif
int netif_rx(struct sk_buff *skb)
{
	struct ethhdr *eth;
	int proto;

	eth = (struct ethhdr *)skb->data_buf;

	switch (ntohs(eth->h_proto))
	{
	case ETH_P_IP:
		return ip_process(skb);
		break;
	case ETH_P_ARP:
		return arp_process(skb);
		break;
	default:
		free_skbuff(skb);
		break;
	}
	return 0;
}

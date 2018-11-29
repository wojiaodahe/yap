#include "arp.h"
#include "eth.h"
#include "error.h"
#include "timer.h"
#include "common.h"
#include "ip.h"
#include "inet.h"
#include "printk.h"
#include "syslib.h"


void print_arp(	struct arp_hdr *arp)
{
	int i;
	printk("----------arp process----------\n");
	printk("type: %d\n", ntohs(arp->type));

	printk("smac: ");
	for(i = 0; i < 6; i++)
		printk("%x ", arp->smac[i]);
	printk("\n");

	printk("dmac: ");
	for(i = 0; i < 6; i++)
		printk("%x ", arp->dmac[i]);
	printk("\n");

	printk("daddr: %x\n", ntohl(arp->daddr));
	printk("saddr: %x\n", ntohl(arp->saddr));

	printk("protocol: %x\n", ntohs(arp->protocol));
	printk("protolen: %x\n", arp->protolen);
	printk("hwlen: %x\n", arp->hwlen);
}

void arp_queue_tick(void *data);
static struct arp_table ARP_TABLE[MAX_ARP_TABLE_NUM];
static struct list_head arp_send_q_head;
static struct timer_list arp_send_q_timer =
{
    .expires = 5,
    .data = 0,
    .function = arp_queue_tick,
};

int add_arp_table(unsigned char *mac, unsigned int ip, struct net_device *ndev)
{
	int i;

	if (!mac || !ip)
		return -EINVAL;

	for (i = 0; i < MAX_ARP_TABLE_NUM; i++)
	{
		if (ARP_TABLE[i].ip == 0)
		{
			memcpy(ARP_TABLE[i].mac, mac, 6);
			ARP_TABLE[i].ip = ip;
			ARP_TABLE[i].ndev = ndev;
			return 0;
		}
	}
	return -ENOMEM;
}

void delete_arp_table()
{

}

struct arp_table *search_arp_table(unsigned int ip)
{
	int i;

	for (i = 0; i < MAX_ARP_TABLE_NUM; i++)
	{
		if (ARP_TABLE[i].ip == ip)
			return &ARP_TABLE[i];
	}

	return NULL;
}


void arp_send_q(void)
{
    struct list_head *list;
    struct sk_buff *skb;
    struct iphdr *iph;
    struct arp_table *arpt;
    struct ethhdr *eth;


    if (list_empty(&arp_send_q_head))
        return;
//disable_irq();
    list = arp_send_q_head.next;
    while (list != &arp_send_q_head)
    {
        skb = list_entry(list, struct sk_buff, list);
        list = list->next;
        iph = (struct iphdr *)(skb->data_buf + sizeof (struct ethhdr));
        arpt = search_arp_table(iph->daddr);
        if (arpt)
        {
            list_del(&skb->list);
            eth = (struct ethhdr *)skb->data_buf;
            memcpy(eth->h_dest, arpt->mac, 6);
            arpt->ndev->hard_start_xmit(skb, arpt->ndev);
            continue;
        }

        if (skb->timeout)
        	skb->timeout--;

		if (!skb->timeout)
		{
			list_del(&skb->list);
			free_skbuff(skb);
		}
    }
}

void arp_queue_tick(void *data)
{
    arp_send_q();
    if (!list_empty(&arp_send_q_head))
        mod_timer(&arp_send_q_timer, 5);
}

void add_skb_to_arp_send_q(struct sk_buff *skb)
{
    skb->timeout = 20;
    list_add_tail(&skb->list, &arp_send_q_head);
    mod_timer(&arp_send_q_timer, 5);
}

int arp_send_request(unsigned int ip, struct net_device *ndev)
{
	unsigned char mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct sk_buff *skb;
	struct arp_hdr *arph;
	struct ethhdr *eth;

	skb = alloc_skbuff(128);
	if (!skb)
		return -ENOMEM;

	eth = (struct ethhdr *)(skb->data_buf);
	memcpy(eth->h_dest, mac, 6);
	memcpy(eth->h_source, ndev->macaddr, 6);
	eth->h_proto = htons(ETH_P_ARP);

	arph = (struct arp_hdr *)(skb->data_buf + sizeof (struct ethhdr));
	arph->daddr 	= ip;
	arph->saddr 	= htonl(ndev->ip);
	arph->type  	= (htons(ARP_REQ));
	arph->protocol 	= (htons(ETH_P_IP));
	arph->hwlen 	= 6;
	arph->protolen 	= 4;
	arph->hwtype 	= htons(ARP_HW_TYPE_ETH);
	memcpy(arph->smac, ndev->macaddr, 6);
	memcpy(arph->dmac, mac, 6);

	skb->data_len	= 42;
	return ndev->hard_start_xmit(skb, ndev);
}

int arp_process(struct sk_buff *skb)
{
	struct ethhdr  *eth;
	struct arp_hdr *arp;
	int ret;

	arp = (struct arp_hdr *)(skb->data_buf + sizeof (struct ethhdr));
	eth = (struct ethhdr *)(skb->data_buf);
	
	switch (ntohs(arp->type))
	{
	case ARP_REQ:
		if ((ret = add_arp_table(arp->smac, arp->saddr, skb->ndev)) <0)
		{
			printk("arp table is full\n");
		}
		if (ntohl(arp->daddr) == skb->ndev->ip)
		{
            memcpy(eth->h_dest, arp->smac, 6);
            memcpy(eth->h_source, skb->ndev->macaddr, 6);
            eth->h_proto = htons(ETH_P_ARP);

			arp->type = htons(ARP_RSP);
            memcpy(arp->dmac, arp->smac, 6);
            memcpy(arp->smac, skb->ndev->macaddr, 6);
            arp->daddr = arp->saddr;
            arp->saddr = htonl(skb->ndev->ip);
            skb->data_len= 42;
            skb->ndev->hard_start_xmit(skb, skb->ndev);
		}
        else
            free_skbuff(skb);
		arp_send_q();
		break;
	case ARP_RSP:
		if ((ret = add_arp_table(arp->smac, arp->saddr, skb->ndev)) <0)
		{
			printk("arp table is full\n");
		}
		free_skbuff(skb);
		arp_send_q();
		break;
	default:
		free_skbuff(skb);
		break;
	}

	return ret;
}

void updata_arp_table(struct sk_buff *skb)
{
	struct ethhdr  *eth;
    struct arp_table *arpt;
    struct iphdr *iph;

	eth = (struct ethhdr *)(skb->data_buf);

	switch (ntohs(eth->h_proto))
	{
	case ETH_P_IP:
        iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);
        arpt = search_arp_table(iph->saddr);
        if (arpt)
            return;
        add_arp_table(eth->h_source, iph->saddr, skb->ndev);
        break;
	case ETH_P_ARP:
        break;
	}
}

int arp_send_q_init()
{
    INIT_LIST_HEAD(&arp_send_q_head);
    return add_timer(&arp_send_q_timer);
}

int arp_init(void)
{
    return arp_send_q_init();
}


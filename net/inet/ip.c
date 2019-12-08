#include "ip.h"
#include "arp.h"
#include "eth.h"
#include "udp.h"
#include "tcp.h"
#include "icmp.h"
#include "error.h"
#include "common.h"
#include "netdevice.h"
#include "inet.h"
#include "printk.h"
#include "lib.h"
#include "timer.h"

static unsigned short ip_data_id = 1000;
static struct ipfrag ip_frag_list_head[IP_FRAG_LIST_HEAD_NUM];

void ip_timer_tick(void *data);
static struct timer_list ip_timer = 
{
    .expires   = IP_TICKS,
    .data      = 0,
    .function  =  ip_timer_tick,
};

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

static struct ipfrag *add_frag_list(struct sk_buff *skb, struct iphdr *iph)
{
    int i;

    for (i = 0; i < IP_FRAG_LIST_HEAD_NUM; i++)
    {
        if (ip_frag_list_head[i].id == IP_FRAG_LIST_NO_USED)
        {
            ip_frag_list_head[i].id = ntohs(iph->id);
            list_add_tail(&skb->list, &ip_frag_list_head[i].head);
            ip_frag_list_head[i].timeout = OS_Get_Kernel_Ticks();
            return &ip_frag_list_head[i];
        }
    }

    return NULL;
}

static void free_frag_list(struct ipfrag *frag_head)
{
    struct list_head *list;
    struct sk_buff *skb;

    list = frag_head->head.next;

    while (list != &frag_head->head)
    {
        skb = list_entry(list, struct sk_buff, list);
        list = list->next;
        free_skbuff(skb);
    }

    memset(frag_head, 0, sizeof (struct ipfrag));
}

/* ip层定时器节拍  */
void ip_timer_tick(void *data)
{
    int i;
    unsigned long tick;
    
    tick = OS_Get_Kernel_Ticks();
    for (i = 0; i < IP_FRAG_LIST_HEAD_NUM; i++)
    {
        if ((ip_frag_list_head[i].id != IP_FRAG_LIST_NO_USED) && \
            ((tick - ip_frag_list_head[i].timeout) > IP_FRAG_LIFETIME))/* 如果ip分片的存在时间超过了生存周期,则认为超时*/
            free_frag_list(&ip_frag_list_head[i]);
    }

    mod_timer(&ip_timer, IP_TICKS);
}

extern struct net_device *return_ndev(void);
struct net_device *ip_route(struct ip_addr *dest)
{
	return return_ndev();
}

int ip_do_send(struct sk_buff *skb, struct ip_addr *dest, unsigned char proto, struct net_device *ndev)
{
    int ret;
	struct ethhdr    *eth;
	struct arp_table *arpt = NULL;

	eth = (struct ethhdr *)(skb->data_buf);

	skb->ndev = ndev;
	arpt = search_arp_table(dest->addr);
	if (!arpt)
	{
		ret = arp_send_request(dest->addr, ndev);
        if (ret < 0)
            return ret;
		add_skb_to_arp_send_q(skb);
		return 0;
	}

	//copy smac
	memcpy(eth->h_dest, arpt->mac, 6);
	memcpy(eth->h_source, skb->ndev->macaddr, 6);
	eth->h_proto = htons(ETH_P_IP);

    return netif_tx_queue(skb->ndev, skb);
}

int ip_fragment_create()
{
	return 0;
}

int ip_do_recv(struct sk_buff *skb)
{
	struct iphdr *iph;

	iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);
	
    switch (iph->protocol)
	{
	case PROTO_ICMP:
		return icmp_process(skb);
	case PROTO_UDP:
		return udp_process(skb);
	case PROTO_TCP:
        return tcp_process(skb);
	default:
		free_skbuff(skb);
        return -EPROTONOSUPPORT;
	}

	//return 0;
}

int ip_fragment_glue(struct ipfrag *frag_head)
{
    char *src, *dest;
    struct list_head *list;
    struct sk_buff *skb;
    struct sk_buff *frag;
    struct iphdr *iph;

    if (!frag_head)
        return 0;

    /* 根据ip报文的总大小申请一个skb */
    skb = alloc_skbuff(frag_head->original_len);
    if (!skb)
    {
        free_frag_list(frag_head);
        return 0;
    }
    
    dest = skb->data_buf + OFFSET_IPDATA;
    list_for_each(list, &frag_head->head)
    {
        frag = list_entry(list, struct sk_buff, list);
        iph  = (struct iphdr *)(frag->data_buf + OFFSET_IPHDR);
   
        src  = frag->data_buf + OFFSET_IPDATA; /* src指向当前ip分片的ip数据(ip数据是ip头之后的数据) */
        dest = skb->data_buf + OFFSET_IPDATA + (ntohs(iph->frag_off) & IP_FRAG_MASK); /* dest指向当前ip分片中的数据在整个ip报文中的位置 */
        memcpy(dest, src, iph->tot_len); /* 按照分片中的数据大小,把数据复制到相应的位置 */
    }

    free_frag_list(frag_head);

    return ip_do_recv(skb);
}

int ip_try_fragment_glue(struct sk_buff *skb)
{
    struct iphdr  *iph;
    struct ipfrag *frag_head;
    unsigned short int frag_off;

    if (!skb)
        return 0;

    iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);

    frag_off  = ntohs(iph->frag_off);

//    if (frag_off & IP_DF)
//        return xxxx;
    
    frag_head = search_frag_list(ntohs(iph->id)); /* 根据ip头中的id来查找这个ip其他分片所在的链表  */
    if (!frag_head)
    {
        frag_head = add_frag_list(skb, iph); /* 如果查找失败说明这是此ip报文的第一个分片  */
        if (!frag_head)
        {
            free_skbuff(skb);
            return 0;
        }
    }
    
    frag_head->current_len += iph->tot_len; /* 所有已收到分片中的数据总和加上当前分片的数据大小*/
    
    if (!(frag_off & IP_MF))  /* 如果是最后一个分片,则根据它的偏移和数据大小算出整个ip报文的大小 */
        frag_head->original_len = (frag_off & IP_FRAG_MASK) + iph->tot_len;

    if (frag_head->current_len == frag_head->original_len)  /* 如果已接收的数据和总大小相同说明所有分片已到达,则重组分片 */
        return ip_fragment_glue(frag_head);

	return 0;
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
		iphnew->check    = __inet_chksum((char *)iphnew, SIZEOF_IPHDR);

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
	iph->check    = __inet_chksum((char *)iph, SIZEOF_IPHDR);
	
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
    printk("flag: %x\n", ntohs(iph->frag_off) & (IP_MF | IP_DF | IP_CE));
	printk("frag_off: %x %d\n", ntohs(iph->frag_off) & IP_FRAG_MASK, ntohs(iph->frag_off) & IP_FRAG_MASK);
	printk("protocol: %x\n", iph->protocol);
	printk("saddr: %x\n", ntohl(iph->saddr));
	printk("daddr: %x\n", ntohl(iph->daddr));
}

int ip_recv(struct sk_buff *skb)
{
	struct iphdr *iph;
    unsigned short frag_off;

	iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);
    frag_off = ntohs(iph->frag_off);

	if ((frag_off & IP_MF) || (frag_off & IP_FRAG_MASK))
		return ip_try_fragment_glue(skb);

	return ip_do_recv(skb);
}

void ip_init(void)
{
    int i;

    for (i = 0; i < IP_FRAG_LIST_HEAD_NUM; i++)
    {
        ip_frag_list_head[i].id              = IP_FRAG_LIST_NO_USED ;
        ip_frag_list_head[i].skb             = NULL;
        ip_frag_list_head[i].current_len     = 0;
        ip_frag_list_head[i].original_len    = 0;
        ip_frag_list_head[i].timeout         = 0;
        INIT_LIST_HEAD(&ip_frag_list_head[i].head);
    }

    add_timer(&ip_timer);
}
    

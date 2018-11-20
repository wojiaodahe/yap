#include "socket.h"
#include "netdevice.h"
#include "inet_socket.h"
#include "eth.h"
#include "ip.h"
#include "udp.h"
#include "error.h"
#include "config.h"
#include "common.h"
#include "tcp.h"

static struct udp udp_poll[MAX_UDP_LINK_NUM];

struct udp *alloc_udp(struct i_socket *isk)
{
	int i;

	for (i = 0; i < MAX_UDP_LINK_NUM; i++)
	{
		if (!(udp_poll[i].flags & UDP_IS_USED))
		{
			memset(&udp_poll[i], 0, sizeof (struct udp));
			udp_poll[i].isk = isk;
			return &udp_poll[i];
		}
	}
	
	return NULL;
}

void free_udp(struct udp *udp)
{
	if (udp)
		udp->flags &= ~UDP_IS_USED;
}

struct udp *search_udp_use_local_port(unsigned short port)
{
	int i;
	
	for (i = 0; i < MAX_UDP_LINK_NUM; i++)
	{
		if (udp_poll[i].local_port == port)
			return &udp_poll[i];
	}
	
	return NULL;
}

int udp_send(struct i_socket *isk, unsigned char *buf, int len, int nonblock, unsigned flags)
{
	return 0;
}

short int udp_check(struct ip_addr *sip, struct ip_addr *dip, unsigned char *udp_buffer, unsigned short udp_size)
{
    struct udp_pseudo_hdr *phead;
    short int check_sum = 0;         //校验和字段置零；原来程序中定义为unsigned long

    phead = (struct udp_pseudo_hdr *)(udp_buffer - sizeof (struct udp_pseudo_hdr));      //缓存数组转换成结构体指针
    phead->sip   = sip->addr;
    phead->dip   = dip->addr;
    phead->mbz   = 0;
    phead->proto = 0x11;          //UDP协议代码为17
    phead->len   = htons(udp_size);

    check_sum = inet_chksum(phead, sizeof (struct udp_pseudo_hdr) + udp_size);

    return check_sum;
}

#define DATA_LEN	2000
int test_udp_send()
{
	struct sk_buff *skb;
	struct udphdr  *udph;
	unsigned short dport = 8000;
	unsigned short sport = 51291;
	struct ip_addr dest;
	struct ip_addr src;
	char *data;

	dest.addr = htonl(0xc0a80108);
	src.addr  = htonl(0xc0a80150);
	skb = alloc_skbuff(DATA_LEN + SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR);
	if (!skb)
	{
		printk("No More Skb!!\n");
		panic();
	}

	udph         = (struct udphdr *)(skb->data_buf + OFFSET_UDPHDR);
	udph->dest   = htons(dport);
	udph->source = htons(sport);
	udph->len 	 = htons(DATA_LEN + SIZEOF_UDPHDR);
	udph->check  = 0;
	udph->check  = udp_check(&src, &dest, udph, DATA_LEN +SIZEOF_UDPHDR);
	skb->data_len = DATA_LEN + SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR;
	data = skb->data_buf + OFFSET_UDPDATA;
	ip_send(skb, &dest, 0x11);
}

int udp_recv(struct i_socket *isk, char *buf, int len, int nonblock, unsigned flags)
{
	struct sk_buff *tmp;
	struct list_head *list;

	if (!isk)
		return -EBADF;

	wait_event(&isk->wq, (isk->flags & PACKET_RECVED));
	return 0;
}

int udp_sendto(struct i_socket *isk, unsigned char *buf, int len, int noblock,
				   unsigned flags, struct sockaddr_in *usin, int addr_len)
{
	struct udp *udp;
	struct sk_buff *skb;
	struct udphdr  *udph;
	struct ip_addr dest;
	struct ip_addr src;
	struct net_device *ndev; 
	

	dest.addr = usin->sin_addr.addr;
	ndev = ip_route(&dest);	
	if (!ndev)
	{
		//no route to dest ip
		return -ENETUNREACH;
	}

	skb = alloc_skbuff(len + SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR);
	if (!skb)
	{
		printk("No More Skb!!\n");
		panic();
	}

	src.addr = htonl(ndev->ip);
	udp = (struct udp *)isk->prot;
	if (!udp)
		return -EBADF;

	if (!udp->remote_port)
		udp->remote_port = usin->sin_port;
	
	udph		  = (struct udphdr *)(skb->data_buf + OFFSET_UDPHDR);
	udph->dest	  = htons(udp->local_port);
	udph->source  = htons(udp->remote_port);
	udph->len	  = htons(len + SIZEOF_UDPHDR);
	udph->check   = 0;
	udph->check   = udp_check(&src, &dest, udph, len + SIZEOF_UDPHDR);
	skb->data_len = len + SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR;

	return ip_send(skb, &dest, INET_PROTO_UDP);
}

int udp_recvfome(struct i_socket *isk, void *ubuf, int len, int noblock,
		      unsigned flags, struct sockaddr *sin, int *addrlen)
{
	int len_remain = len;
	struct sk_buff *tmp;
	struct list_head *list;
	char *buf;
	
	if (!isk)
		return -EBADF;

	if (!ubuf)
		return -EINVAL;

	if (!sin)
		return -EINVAL;

	if (noblock && (!(isk->flags & PACKET_RECVED)))
		return -EAGAIN;
	
	wait_event(&isk->wq, (isk->flags & PACKET_RECVED));

	buf = ubuf;
	disable_irq();
	list_for_each(list, &isk->recv_data_head)
	{
		tmp = list_entry(list, struct sk_buff, list);

		if (len_remain >= tmp->data_len)
		{
			len_remain -= tmp->data_len;
			memcpy(buf, tmp->data_buf, tmp->data_len);
			buf += tmp->data_len;
		}
		else if (len_remain > 0)
		{
			memcpy(buf, tmp->data_buf, len_remain);
			len_remain = 0;
			break;
		}
	}

	free_all_skb(&isk->recv_data_head);
	isk->flags &= ~(PACKET_RECVED);
	enable_irq();

	return (len - len_remain);
}

int udp_recv_callback(struct sk_buff *skb)
{
	struct udphdr  *udph;
	struct udp *udp;
	struct i_socket *isk;
	
	udph = (struct udphdr *)(skb->data_buf + OFFSET_UDPHDR);

	udp = search_udp_use_local_port(udph->dest);
	if (!udp)
	{
		free_skbuff(skb);		
		return -ENETUNREACH;
	}

	isk = udp->isk;
	isk->errno = 0;
	list_add_tail(&skb->list, &udp->isk->recv_data_head);
	udp->isk->flags |= PACKET_RECVED;
	wake_up(&udp->isk->wq);

	return 0;
}

int udp_process(struct sk_buff *skb)
{
	return udp_recv_callback(skb);
}
			  

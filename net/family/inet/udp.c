#include "socket.h"
#include "interrupt.h"
#include "netdevice.h"
#include "inet_socket.h"
#include "eth.h"
#include "ip.h"
#include "udp.h"
#include "error.h"
#include "config.h"
#include "common.h"
#include "inet.h"
#include "printk.h"
#include "syslib.h"
#include "inet.h"

static struct list_head udp_queue;

struct i_socket *find_udp_use_local_port(unsigned short port)
{
    struct list_head *list;
    struct i_socket  *isk;

    list = udp_queue.next;

    while (list != &udp_queue)
    {
        isk = list_entry(list, struct i_socket, list);
        if (isk->local_port == port)
            return isk;

        list = list->next;
    }

    return NULL;
}

unsigned short int udp_get_an_unused_port(void)
{
    static unsigned short port = 10000;

    return port++;
}

void udp_new_sock(struct i_socket *isk)
{
    if (!isk)
        return;

    isk->local_port = udp_get_an_unused_port();

    list_add_tail(&isk->list, &udp_queue);
}

int udp_send(struct i_socket *isk, unsigned char *buf, int len, int nonblock, unsigned flags)
{
	return 0;
}

#if 0
short int udp_check(struct ip_addr *sip, struct ip_addr *dip, unsigned char *udp_buffer, unsigned short udp_size)
{
    struct udp_pseudo_hdr *phead;
    short int check_sum = 0;         //校验和字段置零；原来程序中定义为unsigned long

    phead = (struct udp_pseudo_hdr *)(udp_buffer - sizeof (struct udp_pseudo_hdr));      //缓存数组转换成结构体指针
    phead->sip   = sip->addr;
    phead->dip   = dip->addr;
    phead->mbz   = 0;
    phead->proto = 0x6;          //UDP协议代码为17
    phead->len   = htons(udp_size);

    check_sum = inet_chksum(phead, sizeof (struct udp_pseudo_hdr) + udp_size);

    return check_sum;
}
#endif

#define DATA_LEN	200
int test_udp_send()
{
	struct sk_buff *skb;
	struct udphdr  *udph;
	unsigned short dport = 8000;
	unsigned short sport = 51291;
	struct ip_addr dest;
	struct ip_addr src;

	dest.addr = htonl(0xc0a80168);
	src.addr  = htonl(0xc0a80150);
	skb = alloc_skbuff(DATA_LEN + SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR);
	if (!skb)
	{
		printk("%s No More Skb!!\n", __func__);
		panic();
	}

	udph         = (struct udphdr *)(skb->data_buf + OFFSET_UDPHDR);
	udph->dest   = htons(dport);
	udph->source = htons(sport);
	udph->len 	 = htons(DATA_LEN + SIZEOF_UDPHDR);
	udph->check  = 0;
	udph->check  = inet_check(&src, &dest, (unsigned char *)udph, DATA_LEN + SIZEOF_UDPHDR, INET_PROTO_UDP);
	skb->data_len = DATA_LEN + SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR;
    return ip_send(skb, &dest, 0x11);
}


int udp_recv(struct i_socket *isk, char *buf, int len, int nonblock, unsigned flags)
{
	return 0;
}

int udp_sendto(struct i_socket *isk, char *buf, int len, int noblock,
				   unsigned flags, struct sockaddr_in *usin, int addr_len)
{
	struct sk_buff *skb;
	struct udphdr  *udph;
	struct ip_addr *dest;
	struct ip_addr src;
	struct net_device *ndev; 
	
    if (!isk)
        return -EBADF;

	dest = &usin->sin_addr;

	ndev = ip_route(dest);	
	if (!ndev)
		return -ENETUNREACH;
	
    src.addr = htonl(ndev->ip);

	skb = alloc_skbuff(len + SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR);
	if (!skb)
	{
		printk("%s No More Skb!!\n", __func__);
		panic();
	}

    memcpy(skb->data_buf + OFFSET_UDPDATA, buf, len);

	udph		  = (struct udphdr *)(skb->data_buf + OFFSET_UDPHDR);
	udph->dest	  = usin->sin_port;
	udph->source  = htons(isk->local_port);
	udph->len	  = htons(len + SIZEOF_UDPHDR);
	udph->check   = 0;
	udph->check   = inet_check(&src, dest, (unsigned char *)udph, len + SIZEOF_UDPHDR, INET_PROTO_UDP);
	skb->data_len = len + SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR;

	return ip_send(skb, dest, INET_PROTO_UDP);
}

int udp_recvfrom(struct i_socket *isk, char *ubuf, int len, int noblock,
		      unsigned int flags, struct sockaddr *sin, int *addrlen)
{
	int len_remain = len;
	struct sk_buff *tmp;
	struct list_head *list;
	char *buf;
	
	if (!isk)
		return -EBADF;

	if (!ubuf)
		return -EFAULT;

	if (!sin)
		return -EINVAL;

	if (noblock && (!(isk->flags & PACKET_RECVED)))
		return -EAGAIN;
    
	wait_event(&isk->wq, !list_empty(&isk->recv_data_head));

	buf = ubuf;
	enter_critical();
	list_for_each(list, &isk->recv_data_head)
	{
		tmp = list_entry(list, struct sk_buff, list);

		if (len_remain >= tmp->data_len)
		{
			len_remain -= tmp->data_len;
			memcpy(buf, (tmp->data_buf + OFFSET_UDPDATA), tmp->data_len);
			buf += tmp->data_len;
		}
		else if (len_remain > 0)
		{
			memcpy(buf, (tmp->data_buf + OFFSET_UDPDATA), len_remain);
			len_remain = 0;
			break;
		}
	}

	free_all_skb(&isk->recv_data_head);
    INIT_LIST_HEAD(&isk->recv_data_head);
	exit_critical();

	return (len - len_remain);
}

//修改些函数为udp_recv
int udp_recv_callback(struct sk_buff *skb)
{
	struct udphdr  *udph;
	struct i_socket *isk;
	
	udph = (struct udphdr *)(skb->data_buf + OFFSET_UDPHDR);

	isk = find_udp_use_local_port(ntohs(udph->dest));
	if (!isk)
	{
		free_skbuff(skb);		
		return -ENETUNREACH;
	}

	isk->errno = 0;
	list_add_tail(&skb->list, &isk->recv_data_head);
	wake_up(&isk->wq);

	return 0;
}
	
int  udp_bind(struct i_socket *isk, struct sockaddr_in *usin, int addrlen)
{
    return 0;
}

void print_udp(struct sk_buff *skb)
{
    int i;
    struct udphdr *udph;
    char *data;

    udph = (struct udphdr *)(skb->data_buf + OFFSET_UDPHDR);

    printk("sorce: %d\n", (ntohs(udph->source)));
    printk("dest:  %d\n", (ntohs(udph->dest)));
    printk("len:   %d\n", (ntohs(udph->len)));

#if 0
    data = skb->data_buf + OFFSET_UDPDATA;
    for (i = 0; i < (ntohs(udph->len)); i++)
    {
        //printk("%c ", data[i]);
    }
#endif
}

int udp_process(struct sk_buff *skb)
{
//    print_udp(skb);
	return udp_recv_callback(skb);
}

struct i_proto_opt udp_opt[] =
{
    SOCK_DGRAM,
	NULL,
	NULL,
	NULL,
	udp_sendto,
	udp_recvfrom,
    NULL,
	udp_bind,
	NULL,
	NULL,
    NULL,
    NULL,
    udp_new_sock,
};

int udp_init(void)
{
    INIT_LIST_HEAD(&udp_queue);

    return inet_register_proto(udp_opt);
}

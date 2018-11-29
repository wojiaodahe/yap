#include "inet_socket.h"
#include "error.h"
#include "fs.h"
#include "head.h"
#include "list.h"
#include "socket.h"
#include "wait.h"
#include "common.h"
#include "config.h"
#include "tcp.h"
#include "inet.h"
#include "printk.h"
#include "syslib.h"
#include "ip.h"


static struct list_head active_queue;
static struct list_head listen_queue;
static struct list_head wait_queue;

struct tcp_seg *alloc_tcp_seg()
{
    return NULL;
}

void free_tcp_seg(struct tcp_seg *seg)
{

}

struct i_socket *find_sock(struct sk_buff *skb, struct tcphdr *tcph)
{
    struct list_head *list;
    struct i_socket  *isk;
    struct iphdr     *iph;

    if (!skb || !tcph)
        return NULL;

    iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);

    if ((tcph->syn && tcph->ack) || (!tcph->syn && tcph->ack))
    {
        if (list_empty(&active_queue))
            return NULL;
        list_for_each(list, &active_queue)
        {
            isk = list_entry(list, struct i_socket, list);
            if ((isk->remote_ip == ntohl(iph->saddr)) && \
                (isk->remote_port == ntohs(tcph->source)))
                return isk;
        }
    }
    else if (tcph->syn)
    {
        if (list_empty(&listen_queue))
            return NULL;
        list_for_each(list, &listen_queue)
        {
            isk = list_entry(list, struct i_socket, list);
            if ((isk->local_ip == ntohl(iph->daddr)) && \
                (isk->local_port == ntohs(tcph->dest)))
                return isk;
        }

    }
    else
    {

    }

    return NULL;
}

int tcp_send_seg(struct i_socket *isk, char *buf, unsigned int len)
{
    struct tcp_seg *seg;

    do
    {
        seg = alloc_tcp_seg();
        if (!seg)
            return -EAGAIN;
        
        seg->data = buf;
        if (len >= isk->mss)
        {
            seg->len  = isk->mss;
            len       -= isk->mss;
            buf       += isk->mss;
        }
        else
        {
            seg->len = len;
            len      = 0;
        }

        isk->send_seq += seg->len;
        seg->expected_ack = isk->send_seq;

        list_add_tail(&seg->list, &isk->unsend);
    }while (len > 0);
    
    return 0;
}

int tcp_do_send(struct i_socket *isk, unsigned char *buf, unsigned int len)
{
	return 0;
}

int tcp_send_syn(struct i_socket *isk)
{
	return 0;
}

int tcp_ack_handler(struct i_socket *isk, struct tcphdr *tcph)
{
    struct tcp_seg *seg;
    struct list_head *list;

    if (list_empty(&isk->unacked)) 
        return 0;

    list = isk->unacked.next;
    while (list != &isk->unacked)
    {
        seg = list_entry(list, struct tcp_seg, list);
        if (seg->expected_ack <= isk->received_ack)
        {
            list_del(&seg->list);
            free_tcp_seg(seg);
        }
    }
    return 0;
}

unsigned short tcp_select_window(struct i_socket *isk)
{
    unsigned short window = 2048 * 10;

    return window;
}

unsigned short tcp_check()
{
    return 0;
}

unsigned short tcp_get_mss(void)
{
    return 1460;
}

void tcp_syn_build_options(struct sk_buff *skb, int mss, int ts, int sack, \
                          int offer_wscale, int wacale, unsigned int tstamp, unsigned int ts_recent)
{
    __packed unsigned int *ptr;
    struct tcphdr *tcph;

    tcph = (struct tcphdr *)(skb->data_buf + OFFSET_TCPHDR);
    ptr = (unsigned int *)(tcph + 1);

    *ptr++ = htonl((TCPOPT_MSS << 24) | (TCPOLEN_MSS << 16) | mss);
    if (sack)
    {
        *ptr++ = htonl((TCPOPT_SACK_PERM  << 24) |\
                       (TCPOLEN_SACK_PERM << 16) |\
                       (TCPOPT_TIMESTAMP  << 8)  |\
                       (TCPOLEN_TIMESTAMP));
    }

    if (wacale)
    {
        *ptr++ = htonl((TCPOPT_NOP     << 24) |\
                       (TCPOPT_WINDOW  << 16) |\
                       (TCPOLEN_WINDOW << 8)  |\
                       (wacale));
    }
}

void tcp_build_header(struct sk_buff *skb, struct i_socket *isk, struct tcphdr *tcph, \
                      unsigned int seq, unsigned ack_seq, int ack, int syn, int opt)
{
    struct tcphdr *tcp;
    int doff;

    if (!skb || !isk || !tcph)
        return;

    doff = (opt) ? OPTIONED_TCPHEAD_SIZE : SIZEOF_TCPHDR;
    
    tcp = (struct tcphdr *)(skb->data_buf + OFFSET_TCPHDR);
    tcp->source     = tcph->dest;
    tcp->dest       = tcph->source;
    tcp->ack        = ack;
    tcp->seq        = htonl(seq);
    tcp->ack_seq    = htonl(ack_seq); 
    tcp->window     = htons(tcp_select_window(isk));
    tcp->syn        = syn;
    tcp->doff       = doff / 4;
    
    if (opt)
        tcp_syn_build_options(skb, tcp_get_mss(), TCPOPT_TS_N, TCPOPT_SACK_Y, TCPOPT_WSCALE_Y, TCPOPT_WSCALE_VALUE, 0, 0);
}

#if 0
void tcp_send_ack(struct i_socket *isk, struct tcphdr *tcph, unsigned int seq, unsigned int ack_seq, int syn, int opt)
{
    struct sk_buff *skb;
    struct tcphdr *tcp;
    struct ip_addr dest;
    struct net_device *ndev;
    struct ip_addr src;
    int doff;

    if (!isk || !tcph)
        return;

    doff = (syn) ? OPTIONED_TCPHEAD_SIZE : SIZEOF_TCPHDR;
    skb = alloc_skbuff(SIZEOF_ETHHDR + SIZEOF_IPHDR + doff);
    if (!skb)
        return;
    
    tcp = (struct tcphdr *)(skb->data_buf + OFFSET_TCPHDR);
    tcp->source     = tcph->dest;
    tcp->dest       = tcph->source;
    tcp->ack        = 1;
    tcp->seq        = htonl(seq);
    tcp->ack_seq    = htonl(ack_seq); 
    tcp->window     = htons(tcp_select_window(isk));
    tcp->syn        = syn;
    tcp->doff       = doff / 4;
    
    dest.addr = htonl(isk->remote_ip);
    skb->data_len = SIZEOF_ETHHDR + SIZEOF_IPHDR + doff;

    if (syn)
        tcp_syn_build_options(skb, tcp_get_mss(), 0, 1, 1, 7, 0, 0);
    
    src.addr = htonl(0xc0a80150);
    tcp->check      = 0;
    tcp->check      = inet_check(&src, &dest, (unsigned char *)tcp, doff, INET_PROTO_TCP);
   
    ip_send(skb, &dest, PROTO_TCP);
}

#else
void tcp_send_ack(struct i_socket *isk, struct tcphdr *tcph, \
                  unsigned int seq, unsigned int ack_seq, int syn, int opt)
{
    struct sk_buff *skb;
    struct tcphdr *tcp;
    struct ip_addr dest;
    struct net_device *ndev;
    struct ip_addr src;
    int doff;

    if (!isk || !tcph)
        return;

    doff = (syn) ? OPTIONED_TCPHEAD_SIZE : SIZEOF_TCPHDR;
    skb = alloc_skbuff(SIZEOF_ETHHDR + SIZEOF_IPHDR + doff);
    if (!skb)
        return;
    
    tcp_build_header(skb, isk, tcph, seq, ack_seq, ACK_Y, syn, opt);
    skb->data_len = SIZEOF_ETHHDR + SIZEOF_IPHDR + doff;
    
    dest.addr = htonl(isk->remote_ip);
    src.addr = htonl(isk->local_ip);
    tcp = (struct tcphdr *)(skb->data_buf + OFFSET_TCPHDR);
    tcp->check      = 0;
    tcp->check      = inet_check(&src, &dest, (unsigned char *)tcp, doff, INET_PROTO_TCP);
   
    ip_send(skb, &dest, PROTO_TCP);
}
#endif

void set_tcp_options(struct i_socket *isk, struct tcphdr *tcph)
{

}

int tcp_connect_request_handler(struct sk_buff *skb, struct i_socket *isk, struct tcphdr *tcph)
{
    struct iphdr *iph;

    //if (!skb || !isk || !tcph)
    //   return -XXX;

    iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);

    isk->remote_ip   = ntohl(iph->saddr);
    isk->remote_port = ntohs(tcph->source);

    tcp_send_ack(isk, tcph, 1234, ntohl(tcph->seq) + 1, SYN_Y, OPT_Y);
    isk->status = SYN_RCVD;
    
    list_del(&isk->list);
    list_add_tail(&isk->list, &active_queue);
    
    free_skbuff(skb);
    return 0;
}

void complete_establish(struct i_socket *isk)
{
    if (!isk)
        return;

    isk->status = ESTABLISHED;

    printk("Complete An Establish\n");
    wake_up(&isk->wq);
}

int tcp_data(struct i_socket *isk, struct tcphdr *tcph, struct sk_buff *skb)
{
    return 0;
}

int tcp_fin_handler()
{
    return 0;
}

int tcp_reset(struct i_socket *isk, struct tcphdr *tcph)
{
    return 0;
}

int tcp_urg_handler()
{
    return 0;
}

void print_tcp(struct sk_buff *skb)
{
    __packed unsigned char *ptr;
    int opt_code  = 0;
    int opt_size = 0;
    int length = 0;
    struct tcphdr *tcph;

    tcph = (struct tcphdr *)(skb->data_buf + OFFSET_TCPHDR);
    length = (tcph->doff * 4) - sizeof (struct tcphdr);
    ptr = (unsigned char *)(tcph + 1);

    printk("source:  %d\n", ntohs(tcph->source));
    printk("dest:    %d\n", ntohs(tcph->dest));
    printk("doff:    %d\n", tcph->doff);
    printk("seq:     %d\n", ntohl(tcph->seq));
    printk("ack_seq: %d\n", ntohl(tcph->ack_seq));
    printk("syn:     %d\n", tcph->syn);
    printk("ack:     %d\n", tcph->ack);
    printk("window:  %d\n", ntohs(tcph->window));
    printk("check:   %d\n", ntohs(tcph->check));
    
    while (length > 0)
    {
        while (length > 0 && *ptr == TCPOPT_NOP)
        {
            length--;
            ptr++;
        }
        
        if (!length)
            return;

        opt_code = *ptr++;
        opt_size = *ptr++;

        switch (opt_code)
        {
        case TCPOPT_EOL:
            return;
        case TCPOPT_MSS:
            printk("TCPOPT_MSS: %d\n", ntohs(*(unsigned short *)ptr));
            break;
        case TCPOPT_SACK_PERM:
            printk("TCPOPT_SACK\n");
            break;
        case TCPOPT_WINDOW:
            printk("TCPOPT_WINDOW: %d\n", *ptr);
            break;
        }
        ptr += (opt_size - 2);
        length -= opt_size;
    }
}

int tcp_received_data_handler(struct sk_buff *skb)
{
	struct tcphdr *tcph;
	struct i_socket *isk;

	if (!skb)
		return 0;
    
    print_tcp(skb);
	
	tcph = (struct tcphdr *)(skb->data_buf + OFFSET_TCPHDR);
	isk = find_sock(skb, tcph);

	if (!isk)
	{
		free_skbuff(skb);
		return 0;///xxxxxxxxxxxxxxxxxxxxxxxx
	}
	//list_add_tail(&skb->list, &isk->back_log);

    printk("------------isk(%x)->status: %d--------------\n", isk, isk->status);
	switch (isk->status)
	{
	case LAST_ACK:
		isk->errno 	= -ECONNRESET;
		isk->status 	= CLOSED;
		free_skbuff(skb);
		free_isock(isk);
		return 0;
	case ESTABLISHED:
	case CLOSE_WAIT:
	case FIN_WAIT_1:
	case FIN_WAIT_2:
	case TIME_WAIT:
		if (tcph->rst)
		{
			isk->errno = -ECONNRESET;
			if (isk->status == CLOSE_WAIT)
				isk->errno = -EPIPE;
			isk->status = CLOSED;
			free_skbuff(skb);
			free_isock(isk);
			return 0;
		}
		
		if (tcph->syn)
		{
			isk->errno = -ECONNRESET;
			isk->status = CLOSED;
			free_skbuff(skb);
			free_isock(isk);
			return 0;
		}

		if (tcph->ack && (tcp_ack_handler(isk, tcph) < 0))
		{
			free_skbuff(skb);
			free_isock(isk);
			return 0;
		}
		
		if (tcph->urg && (tcp_urg_handler() < 0))
		{
			free_skbuff(skb);
			free_isock(isk);
			return 0;
		}
		
		if (tcph->fin && (tcp_fin_handler() < 0))
		{
			free_skbuff(skb);
			free_isock(isk);
			return 0;
		}

		if (tcp_data(isk, tcph, skb) < 0)
		{
			free_skbuff(skb);
			free_isock(isk);
			return 0;
		}
		break;
	case SYN_SENT:
		if (tcph->rst)
		{
			isk->errno = -ECONNRESET;
			isk->status = CLOSED;
			free_skbuff(skb);
			return 0;
		}

		if (tcp_ack_handler(isk, tcph) < 0)
		{
			tcp_reset(isk, tcph);
			free_skbuff(skb);
			return 0;
		}

		if (!tcph->syn)
		{
			free_skbuff(skb);
			return 0;
		}
		set_tcp_options(isk, tcph);
		isk->status = ESTABLISHED;
		break;
	case SYN_RCVD:
		if (tcp_ack_handler(isk, tcph) < 0)
		{
			tcp_reset(isk, tcph);
			free_skbuff(skb);
			return 0;
		}
		if (tcph->syn)
		{
			tcp_reset(isk, tcph);
			free_skbuff(skb);
			return 0;
		}
		set_tcp_options(isk, tcph);
		isk->status = ESTABLISHED;
        complete_establish(isk);
		break;
	case CLOSED:
		break;
	case LISTEN:
		if (tcph->rst)
		{
			free_skbuff(skb);
			return 0;
		}

		if (tcph->ack)
		{
			free_skbuff(skb);
			return 0;
		}

		if (tcph->syn)
			return tcp_connect_request_handler(skb, isk, tcph);
		else
		{
			free_skbuff(skb);
			return 0;
		}
	}

	return 0;
}

int tcp_process(struct sk_buff *skb)
{
    return tcp_received_data_handler(skb);
}

void tcp_connect_syn_timer_callback(void *data)
{
	struct i_socket *isk;
	int ret;

 	isk = data;
	if (!isk)
		return;

	if (isk->status != ESTABLISHED)
	{
		if (isk->retries)
		{
			isk->retries--;
			ret = tcp_send_syn(isk);
			if (ret == 0)
			{
				ret = mod_timer(&isk->timer, 3);
				if (ret == 0)
					return;
				else
				{
					isk->errno = ret;
					isk->status = CLOSED;
				}
			}
			else
				isk->status = CLOSED;
		}
		else
		{
			isk->errno = -ETIMEDOUT;
			isk->status = CLOSED;
		}
	}
	else 
		isk->errno = 0;
	
	del_timer(&isk->timer);
	wake_up(&isk->wq);
}

int tcp_do_connect(struct i_socket *isk)
{
	int ret;

	isk->timer.expires = 3;
	isk->timer.data    = isk;
	isk->timer.function = tcp_connect_syn_timer_callback;

	ret = tcp_send_syn(isk);
	if (ret < 0)
		return ret;
	
	isk->status = SYN_SENT;
	ret = add_timer(&isk->timer);
	if (ret < 0)
		return ret;
	
	wait_event(&isk->wq, isk->status != SYN_SENT);
		
	return isk->errno;
}


void tcp_close(struct i_socket *isk, int timeout)
{

}

int	tcp_read(struct i_socket *isk, char *to, int len, int nonblock, unsigned flags)
{
	return 0;
}

int	tcp_write(struct i_socket *isk, char *to,  int len, int nonblock, unsigned flags)
{
	return 0;
}

int tcp_sendto(struct i_socket *isk, char *buf, int len, int noblock,
			  unsigned flags, struct sockaddr_in *usin, int addr_len)
{
	return 0;
}

int	tcp_recvfrom(struct i_socket *isk, char *buf, int len, int noblock,
				unsigned flags, struct sockaddr_in *usin, int *addr_len)
{
	return 0;
}

int tcp_connect(struct i_socket *isk, struct sockaddr_in *sin, int addr_len)
{
	return 0;
}

int tcp_bind(struct i_socket *isk, struct sockaddr_in *sin, int addrlen)
{
    if (!isk)
        return -EBADF;

    //if (!sin)
        //return xxx

    isk->local_port = ntohs(sin->sin_port);
    isk->local_ip   = ntohl(sin->sin_addr.addr);
	return 0;	
}


int	tcp_recv(struct i_socket *isk, char *buf, int len, int nonblock, unsigned flags)
{
	return 0;
}

int tcp_listen(struct i_socket *isk, int backlog)
{
    if (isk->status != CLOSED)	
        return -EINVAL;

    isk->status = LISTEN;
    isk->backlog = backlog;
    
    /* 此isk一直存在于原始的struct socket中,无需从listen_queue中查找,
     * 所以不和放入listen_queue队列中,此队列用于存放调用了accept之后
     * 的new_isk,详见tcp_accept函数
     **/
    //list_add_tail(&isk->list, &listen_queue);
    
    return 0;
}

int tcp_send(struct i_socket *isk, char *buf, int len, int nonblock, unsigned flags)
{
	return 0;
}

int tcp_accept(struct i_socket *isk, struct sockaddr_in *sin, int *addrlen)
{
    struct i_socket *new_isk;

    new_isk = alloc_isocket();
    if (!new_isk)
        return -EAGAIN;

    new_isk->prot_type      = isk->prot_type;
    new_isk->prot           = isk->prot;
    new_isk->i_prot_opt     = isk->i_prot_opt;
    new_isk->priv           = isk->priv;
    new_isk->backlog        = isk->backlog;
    new_isk->mss            = isk->mss;
    new_isk->mtu            = isk->mtu;
    new_isk->window         = isk->window;
    new_isk->local_ip       = isk->local_ip;
    new_isk->local_port     = isk->local_port;
    new_isk->ssthresh       = isk->ssthresh;
    new_isk->retries        = isk->retries;
    new_isk->status         = isk->status;

    list_add_tail(&new_isk->list, &listen_queue);

    return new_isk;
}

struct i_proto_opt tcp_opt[] =
{
    SOCK_STREAM,
	tcp_close,
	tcp_read,
	tcp_write,
	tcp_sendto,
	tcp_recvfrom,
	tcp_connect,
	tcp_bind,
	tcp_accept,
	tcp_listen,
	tcp_send,
	tcp_recv
};

int tcp_init(void)
{
    INIT_LIST_HEAD(&active_queue);
    INIT_LIST_HEAD(&listen_queue);
    INIT_LIST_HEAD(&wait_queue);
    
    return inet_register_proto(&tcp_opt);
}

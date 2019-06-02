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
#include "kernel.h"
#include "syslib.h"
#include "kmalloc.h"
#include "interrupt.h"


static struct list_head active_queue;
static struct list_head listen_queue;
static struct list_head wait_queue;

struct tcp_seg *alloc_tcp_seg(unsigned int len)
{
    struct tcp_seg *seg;

    seg = kmalloc(sizeof (struct tcp_seg));
    if (!seg)
        return NULL;

    seg->data = kmalloc(len);
    if (!seg->data)
    {
        kfree(seg);
        return NULL;
    }

    INIT_LIST_HEAD(&seg->list);
    seg->retries = TCP_SEG_SEND_RETRIES;

    return seg;
}

void free_tcp_seg(struct tcp_seg *seg)
{
    if (!seg)
        return;
    
    if (seg->data)
        kfree(seg->data);

    kfree(seg);
}

unsigned short int tcp_get_an_unused_port(void)
{
    static unsigned short port = 10002;

    return port++;
}


struct i_socket *tcp_find_sock(struct sk_buff *skb, struct tcphdr *tcph)
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

int tcp_seg_if_timeout(struct i_socket *isk, struct tcp_seg *seg)
{
    /*
    if (timeout)
        isk->flags |= TCP_SEG_TIMEOUT_FLAG;
    */
  
    return 0;
}

int tcp_retransmit(struct i_socket *isk, struct tcp_seg *seg)
{
    return 0;
}

int tcp_do_segment(struct i_socket *isk, char *buf, unsigned int len)
{
    unsigned short seg_len;
    char *dest;
    struct tcp_seg *seg;

    dest = buf;
    while (len > 0)
    {
        if (len >= isk->mss)
            seg_len = isk->mss;
        else
            seg_len = len;

        seg = alloc_tcp_seg(seg_len);
        if (!seg)
            return -ENOMEM;
            
        memcpy(seg->data, dest, seg_len);
        dest += seg_len;
        len  -= seg_len;
        
        seg->len = seg_len;
        seg->send_seq = isk->send_seq;
        isk->send_seq += seg->len;
        seg->expected_ack = isk->send_seq;

        if ((isk->send_window.used_size + seg->len) <= isk->send_window.tot_size)
        {
            list_add_tail(&seg->list, &isk->send_window.ready);
            isk->send_window.used_size += seg_len;
        }
        else
            list_add_tail(&seg->list, &isk->send_window.wait);
    }
    
    return 0;
}

int tcp_send_seg(struct i_socket *isk, struct tcp_seg *seg)
{
    struct sk_buff *skb;
    struct tcphdr *tcp;
    struct ip_addr dest;
    struct ip_addr src;
    char *ptr;

    if (!isk || !seg)
        return 0;

    skb = alloc_skbuff(SIZEOF_ETHHDR + SIZEOF_IPHDR + MAX_TCP_HEADER_SIZE + seg->len);
    if (!skb)
        return -ENOMEM;
    
    tcp_build_header(skb, isk, htons(isk->local_port), htons(isk->remote_port), seg->send_seq, isk->ack_seq, ACK_Y, SYN_N, FIN_N);
    tcp_build_options(skb, MSS_N, 0, TS_N, SACK_N, WSCALE_N, 0, 0, 0);

    tcp = get_tcph(skb);
    skb->data_len = SIZEOF_ETHHDR + SIZEOF_IPHDR + tcp->doff * 4 + seg->len;
    ptr = skb->data_buf + SIZEOF_ETHHDR + SIZEOF_IPHDR + tcp->doff * 4;
    memcpy(ptr, seg->data, seg->len);
    
    dest.addr = htonl(isk->remote_ip);
    src.addr = htonl(isk->local_ip);
    tcp->check      = 0;
    tcp->check      = inet_check(&src, &dest, (unsigned char *)tcp, tcp->doff * 4 + seg->len, INET_PROTO_TCP);
   
    return ip_send(skb, &dest, PROTO_TCP);
}

void tcp_process_send_window(struct i_socket *isk)
{
    int ret = 0;
    struct tcp_seg *seg;
    struct list_head *list;

    /* 如果出现超时,则暂停发送数据  */
    if (isk->flags & TCP_SEG_TIMEOUT_FLAG)
        return;
    
    list = isk->send_window.wait.next;
    while (list != &isk->send_window.wait)    
    {
        seg = list_entry(list, struct tcp_seg, list);
        list = list->next;
        if ((isk->send_window.tot_size - isk->send_window.used_size) >= seg->len)
        {
            list_del(&seg->list);
            list_add_tail(&seg->list, &isk->send_window.ready);
            isk->send_window.used_size += seg->len;
        }
        else
            break;
    }

    list = isk->send_window.ready.next;
    while (list != &isk->send_window.ready)    
    {
        seg = list_entry(list, struct tcp_seg, list);
        ret = tcp_send_seg(isk, seg);
        if (ret < 0)
            break;
        list = list->next;
        list_del(&seg->list);
        list_add_tail(&seg->list, &isk->send_window.wait_ack);
    }

}

int tcp_if_timeout(struct i_socket *isk, unsigned int time)
{
    unsigned int tick;

    tick = OS_Get_Kernel_Ticks();
    
    return ((tick - isk->timeout) * HZ) > time;
}

static void tcp_timer_entry(void *data)
{
    int ret;
    struct sk_buff *skb;
    struct list_head *list;
    struct i_socket *isk = data;
    struct ip_addr dest;
    unsigned int tick;

    if (!isk)
        return;
    
    if (!list_empty(&isk->ack_queue))
    {
        list = isk->ack_queue.next;
        skb  = list_entry(list, struct sk_buff, list);
        list = list->next;
        list_del(&skb->list);
        dest.addr = htonl(isk->remote_ip);
        isk->ack_backlog -= 1;
        ip_send(skb, &dest, PROTO_TCP);
    }
    
    tick = OS_Get_Kernel_Ticks();
    switch (isk->status)
    {
    case SYN_RCVD:
        if (tcp_if_timeout(isk, SYN_RCVD_HOLDING_TIME))
        {
            isk->status = LISTEN;
            del_timer(&isk->timer);
            list_del(&isk->list);
            list_add_tail(&isk->list, &listen_queue);
        }
        break;
    case SYN_SENT:
        if (isk->retries)
        {
            isk->retries--;
            isk->timeout = tick;
            ret = tcp_send_syn(isk);
            if (ret < 0)
            {
                del_timer(&isk->timer);
                isk->status = CLOSED;
                isk->errno = ret;

                /* xxxxxxxxxxxxxxxxxxxxxxxx
                 * wake_up 上层应用
                 * */
            }
            else
            {
                ret = mod_timer(&isk->timer, 20);
                if (ret < 0)
                {
                    del_timer(&isk->timer);
                    isk->status = CLOSED;
                    isk->errno = ret;
                    /* xxxxxxxxxxxxxxxxxxxxxxxx
                     * wake_up 上层应用
                     * */
                }
            }
        }
        else
        {
            del_timer(&isk->timer);
            isk->errno = -ETIMEDOUT;
            isk->status = CLOSED;
            /* xxxxxxxxxxxxxxxxxxxxxxx
             * wake_up 上层应用
             * */    
        }
        return;
    case ESTABLISHED:
        tcp_process_send_window(isk);
        break;
    case FIN_WAIT_1:
    case CLOSING:
    case LAST_ACK:
        if (tcp_if_timeout(isk, TCP_FIN_WAIT_TIME))
        {
            if (isk->retries == 0)
            {
                isk->errno = -ETIMEDOUT;
                isk->status = CLOSED;
                free_isock(isk);
            }
            else
            {
                isk->retries--;
                isk->timeout = OS_Get_Kernel_Ticks();
                if (tcp_send_fin(isk, isk->send_seq, isk->ack_seq) < 0)
                {
                    isk->errno = -EIO;
                    isk->status = CLOSED;
                    free_isock(isk);
                }
            }
        }
        break;
    case TIME_WAIT:
            /* if (ticks - isk->timeout) > 2 * isk->rto xxxxxxxxxxxxxxxxxxxxxxxx */
        isk->errno = 0;
        isk->status = CLOSED;
        free_isock(isk);
        break;
    default:
        return;
    }

    mod_timer(&isk->timer, 1);
}

int tcp_send_syn(struct i_socket *isk)
{
    unsigned int seq;
    struct sk_buff *skb;
    struct tcphdr *tcph;
    struct ip_addr dest;
    struct ip_addr src;

    skb = alloc_skbuff(SIZEOF_ETHHDR + SIZEOF_IPHDR +  SIZEOF_TCPHDR + 20);
    if (!skb)
        return -ENOMEM;

    seq = tcp_generate_a_seq();
    isk->send_seq = seq;

    tcp_build_header(skb, isk, htons(isk->local_port), htons(isk->remote_port), htonl(seq), 0, ACK_N, SYN_Y, FIN_N);
    tcp_build_options(skb, MSS_Y, 1460, TS_N, SACK_Y, WSCALE_N, 0, 0, 0);

    tcph = get_tcph(skb);
    skb->data_len = (SIZEOF_ETHHDR + SIZEOF_IPHDR + tcph->doff * 4);
    
    dest.addr = htonl(isk->remote_ip);
    src.addr = htonl(isk->local_ip);
    tcph->check      = 0;
    tcph->check      = inet_check(&src, &dest, (unsigned char *)tcph, tcph->doff * 4, INET_PROTO_TCP);

	return ip_send(skb, &dest, PROTO_TCP);
}

int tcp_ack_handler(struct i_socket *isk, struct tcphdr *tcph)
{
    struct tcp_seg *seg;
    struct list_head *list;
    unsigned int received_ack;
    
    received_ack = ntohl(tcph->ack_seq);
    switch (isk->status)
    {
    case LAST_ACK:
        if (received_ack == (isk->send_seq + 1))
        {
            isk->errno = -ECONNRESET;
            isk->status = CLOSED;
        }
        break;
    case FIN_WAIT_1:
        if (received_ack == (isk->send_seq + 1))
        {
            isk->timeout = OS_Get_Kernel_Ticks();
            isk->status = FIN_WAIT_2;
        }
        break;
    default:
        break;
    }
    
    if (isk->received_ack < received_ack)
        isk->received_ack = received_ack;
  
    enter_critical();
    isk->flags &= ~(TCP_SEG_TIMEOUT_FLAG);
    list = isk->send_window.wait_ack.next;
    while (list != &isk->send_window.wait_ack)
    {
        seg = list_entry(list, struct tcp_seg, list);
        list = list->next;
        if (seg->expected_ack <= received_ack)
        {
            list_del(&seg->list);
            isk->send_window.used_size -= seg->len;
            isk->send_window.tot_size   = htons(tcph->window); //xxxxxxxxxxxxxxxxxxxxxxx  tcp_window = window * option
            free_tcp_seg(seg);
        }
    }
    exit_critical();
   
    return 0;
}

unsigned short tcp_select_window(struct i_socket *isk)
{
    if (isk->window == 0)
        isk->window = 4096;
    else if (isk->window < 40960)
        isk->window += 4096;
    
    return isk->window;
}

unsigned short tcp_check()
{
    return 0;
}

unsigned short tcp_get_mss(void)
{
    return 1460;
}

unsigned int tcp_generate_a_seq(void)
{
    return 0x1234;
}

void tcp_build_options(struct sk_buff *skb, int if_mss, int mss, int if_ts, int if_sack, \
                          int if_wscale, int wacale, unsigned int tstamp, unsigned int ts_recent)
{
    __packed unsigned int *ptr;
    struct tcphdr *tcph;
    int doff = SIZEOF_TCPHDR;

    tcph = get_tcph(skb);
    ptr = (unsigned int *)(tcph + 1);
    
    if (if_mss)
    {
        *ptr++ = htonl((TCPOPT_MSS << 24) | (TCPOLEN_MSS << 16) | mss);
       
        doff += 4;
    }
    
    if (if_ts)
    {
        if (if_sack)
        {
            *ptr++ = htonl((TCPOPT_SACK_PERM  << 24) |\
                    (TCPOLEN_SACK_PERM << 16)        |\
                    (TCPOPT_TIMESTAMP  << 8)         |\
                    (TCPOLEN_TIMESTAMP));
        }
        else
        {
			*ptr++ = htonl((TCPOPT_NOP << 24) |
				       (TCPOPT_NOP << 16) |
				       (TCPOPT_TIMESTAMP << 8) |
				       TCPOLEN_TIMESTAMP);
        }
		*ptr++ = htonl(tstamp);		
		*ptr++ = htonl(ts_recent);	
        
        doff += 12;
    }
    else if (if_sack)
    {
		*ptr++ = htonl((TCPOPT_NOP << 24) |\
			       (TCPOPT_NOP << 16)     |\
			       (TCPOPT_SACK_PERM << 8)|\
			       TCPOLEN_SACK_PERM);
        doff += 4;
    }

    if (if_wscale)
    {
        *ptr++ = htonl((TCPOPT_NOP     << 24) |\
                       (TCPOPT_WINDOW  << 16) |\
                       (TCPOLEN_WINDOW << 8)  |\
                       (wacale));
        doff += 4;
    }

    tcph->doff = doff / 4;
}

void tcp_build_header(struct sk_buff *skb, struct i_socket *isk, unsigned short source, \
                      unsigned short dest, unsigned int seq, unsigned ack_seq, int ack, int syn, int fin)
{
    struct tcphdr *tcp;
    int doff;

    if (!skb || !isk)
        return;

    tcp = get_tcph(skb);
   
    tcp->source     = source;
    tcp->dest       = dest;
    tcp->ack        = ack;
    tcp->seq        = htonl(seq);
    tcp->ack_seq    = htonl(ack_seq); 
    tcp->window     = htons(tcp_select_window(isk));
    tcp->syn        = syn;
    tcp->fin        = fin;
}

int tcp_output(struct i_socket *isk, struct sk_buff *skb)
{
    static int times = 0;


    return 0;
}

int tcp_send_ack(struct i_socket *isk, struct tcphdr *tcph, \
                  unsigned int seq, unsigned int ack_seq, int syn, int fin, unsigned char now)
{
    struct sk_buff *skb;
    struct tcphdr *tcp;
    struct ip_addr dest;
    struct net_device *ndev;
    struct ip_addr src;

    if (!isk || !tcph)
        return -ENOTSOCK;

    skb = alloc_skbuff(SIZEOF_ETHHDR + SIZEOF_IPHDR + MAX_TCP_HEADER_SIZE);
    if (!skb)
        return -ENOMEM;
    
    tcp_build_header(skb, isk, tcph->dest, tcph->source, seq, ack_seq, ACK_Y, syn, fin);
    if (syn)
        tcp_build_options(skb, MSS_Y, tcp_get_mss(), TS_N, SACK_Y, WSCALE_N, 0, 0, 0);
    else
        tcp_build_options(skb, MSS_N, 0, TS_Y, SACK_N, WSCALE_N, 0, 0x1234, 0x5678);

    tcp = get_tcph(skb);
    skb->data_len = SIZEOF_ETHHDR + SIZEOF_IPHDR + tcp->doff * 4;
    
    dest.addr = htonl(isk->remote_ip);
    src.addr = htonl(isk->local_ip);
    tcp->check      = 0;
    tcp->check      = inet_check(&src, &dest, (unsigned char *)tcp, tcp->doff * 4, INET_PROTO_TCP);
   
    if (now)
        return ip_send(skb, &dest, PROTO_TCP);
    else
    {
        list_add_tail(&skb->list, &isk->ack_queue);
        isk->ack_backlog += 1;
    }

    return 0;
}

int tcp_send_fin(struct i_socket *isk, unsigned int seq, unsigned int ack_seq)
{
    return 0;
}

void tcp_set_options(struct i_socket *isk, struct sk_buff *skb)
{
    __packed unsigned char *ptr;
    int opt_code  = 0;
    int opt_size = 0;
    int length = 0;
    struct tcphdr *tcph;
    
    tcph = get_tcph(skb);
    
    length = (tcph->doff * 4) - sizeof (struct tcphdr);
    ptr = (unsigned char *)(tcph + 1);

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
            isk->mss = ntohs(*(unsigned short *)ptr);
            break;
        case TCPOPT_SACK_PERM:
            isk->sack = 1;
            break;
        case TCPOPT_WINDOW:
            isk->window_shift = *ptr;
            break;
        }
        ptr += (opt_size - 2);
        length -= opt_size;
    }
}

int tcp_connect_request_handler(struct sk_buff *skb, struct i_socket *isk, struct tcphdr *tcph)
{
    struct iphdr *iph;

    //if (!skb || !isk || !tcph)
    //   return -XXX;

    iph = (struct iphdr *)(skb->data_buf + OFFSET_IPHDR);

    isk->received_ack = 0;
    isk->send_seq     = tcp_generate_a_seq(); 
    isk->remote_ip    = ntohl(iph->saddr);
    isk->remote_port  = ntohs(tcph->source);
    isk->ack_seq      = ntohl(tcph->seq) + 1;
    tcp_set_options(isk, skb);
    isk->send_window.tot_size = ntohs(tcph->window);
    
    isk->timer.data     = isk;
    isk->timer.expires  = 3; /* 我感觉ubuntu的RTO计算的太激进,这里写成2消磨下它的锐气*/
    isk->timer.function = tcp_timer_entry;
    add_timer(&isk->timer);

    isk->timeout = OS_Get_Kernel_Ticks();
    tcp_send_ack(isk, tcph, isk->send_seq, isk->ack_seq, SYN_Y, FIN_N, ACK_NOMAL);
    
    list_del(&isk->list);
    list_add_tail(&isk->list, &active_queue);
    
    return 0;
}

void complete_establish(struct i_socket *isk)
{
    if (!isk)
        return;
    isk->status = ESTABLISHED;
    isk->send_seq += 1;
    
    /* RTT = a * RTT + (1 - a) * M
     * RTO = RTT * b
     * a = 0.9 
     * b = 2
     * M = 最近一次RTT的值
     * 所以第一次计算 RTT = 0.9 * RTT + 0.1 * RTT = RTT
     */   

    isk->rtt = (OS_Get_Kernel_Ticks() - isk->timeout) * (1000 / HZ);
    isk->rto = 2 * isk->rtt;

    /* 回应给对方的ack会放在队列里,合适时再发出,但这个"适合的时间"不
     * 能无限等下去,因为对方会超时,超时时间(RTO)经上面的算法粗略计算
     * 假如超时时间为100ms,每个包的往返时间(RTT)为20ms,在收到对方发送
     * 的第10个包的时候第一个包就会超时(10 * 20 / 2 = 100).所以这个ack
     * 队列最长不能超过9
     * 
     * 然而!!!!
     * 根据上面一系得出这个公式 isk->max_ack_backlog * (RTT / 2) = RTO
     * 得出:isk->max_ack_backlog = RTO * 2 / RTT = (2 * RTT) * 2 / RTT = 4
     * ack队列不能超过4 ?
     * 有缺陷,再改进
     */

    isk->max_ack_backlog = 3;

    //printk("Complete An Establish\n");
    wake_up(&isk->wq);
}

unsigned short tcp_get_data_lenght(struct sk_buff *skb)
{
    struct iphdr  *iph;
    struct tcphdr *tcph;
    
    iph  = get_iph(skb);
    tcph = get_tcph(skb);

    return (ntohs(iph->tot_len) - SIZEOF_IPHDR - tcph->doff * 4);
}

unsigned int tcp_get_ackseq(struct sk_buff *skb)
{
    struct iphdr  *iph;
    struct tcphdr *tcph;
    
    iph  = get_iph(skb);
    tcph = get_tcph(skb);

    return (htonl(tcph->seq) + (ntohs(iph->tot_len) - SIZEOF_IPHDR - tcph->doff * 4));
}

char *tcp_get_data_off(struct sk_buff *skb)
{
    struct tcphdr *tcph;

    tcph = get_tcph(skb);

    return (skb->data_buf + SIZEOF_ETHHDR + SIZEOF_IPHDR + tcph->doff * 4);
}

/* 把收到的数据按顺序插入链表 */
int tcp_seg_sort(struct i_socket *isk, struct sk_buff *skb)
{
    int ret = 0;
    unsigned int seq;
    unsigned int seq_tmp;
    struct sk_buff *skb_tmp;
    struct tcphdr *tcph;
    struct list_head *list;

    if (!isk || !skb)
    {
        printk("%s %d \n", __func__, __LINE__);
        return -ENOTSOCK;
    }

    tcph = get_tcph(skb);
    seq  = ntohl(tcph->seq);  /* 获取当前报文seq  */

    if (seq < isk->ack_seq)
    {
        free_skbuff(skb);
        return 0;
    }
    
    enter_critical();
    list = isk->recv_data_head.prev;
    while (list != &isk->recv_data_head)   /* 查找所有收到的tcp报文段 */
    {
        skb_tmp  = list_entry(list, struct sk_buff, list);
        tcph     = get_tcph(skb_tmp);
        /* 多数情况下报文段是按顺序到达,因此从链表的尾部开始查找
         * 判断当前报文所在的位置.方法：链表按seq的大小有序排列,只需要把当前报文段插入到第一个seq比它小的报文段后面即可
         */
        seq_tmp = ntohl(tcph->seq);
        if (seq_tmp < seq)
            break;
        else if (seq == seq_tmp) 
        {
            free_skbuff(skb);
            return 0;
        }
        else 
            list = list->prev;
    }
    list_add(&skb->list, list);
    isk->recv_data_len += tcp_get_data_lenght(skb);
    exit_critical();

    /* 唤醒上层等待数据的进程  */
    if (isk->recv_data_len)
        wake_up(&isk->wq);
   
    /* 假设握手完成后的数据阶段序列号从1开始,某时刻收到了序列号为4的包
     * 此时tcp接收队列可能分为以下几种情况
     * 1：之前已经收到了seq为 1 2 3 的包
     * ---------------------------------------------
     * |1|2|3|...
     * ---------------------------------------------
     * 此时变量list指向了seq为3的包
     *
     * 2：之前已经收到了seq为 1 2 3 5 6的包
     * ----------------------------------------------
     * |1|2|3| |5|6|...
     * ----------------------------------------------
     * 此时变量list指向了seq为3的包
     * 
     * 3：之前已经收到了seq为 1 2 3 5 6 8的包
     * ----------------------------------------------
     * |1|2|3| |5|6| |8|...
     * ----------------------------------------------
     * 此时变量list指向了seq为3的包
     *
     * 4：之前已经收到了seq为 1 2 的包
     * ----------------------------------------------
     * |1|2|...
     * ----------------------------------------------
     * 此时变量list指向了seq为2的包 
     * 
     * 5：之前已经收到了seq为 2 3 的包
     * ----------------------------------------------
     * | |2|3|...
     * ----------------------------------------------
     * 此时变量list指向了seq为3的包 
     * 
     * 6：之前已经收到了seq为 2 的包
     * ----------------------------------------------
     * | |2| |...
     * ----------------------------------------------
     * 此时变量list指向了seq为2的包 
     *
     * 7：之前未收到任何数据包
     * ----------------------------------------------
     * | | | |...
     * ----------------------------------------------
     * 此时变量list指向了isk->recv_data_head
     *
     * 第1种情况只需要回一个ack为5的包即可
     * 第2、3种情况需要回ack为5 6 7的三个包
     * 第4种情况需要回ack为3的包
     * 第5、6、7种情况需要回ack为1的包
     */
   
    enter_critical();
    list = list->next;  /*此时list指向新到的这个tcp报文,即skb */
    while (list != &isk->recv_data_head)
    {
        skb_tmp = list_entry(list, struct sk_buff, list);
        tcph = get_tcph(skb_tmp);

        /* 判断这个报文的seq是否是期待的seq  isk->ack_seq在三次握手之后被赋值*/
        if (isk->ack_seq != ntohl(tcph->seq)) 
        {
            ret = tcp_send_ack(isk, tcph, isk->send_seq, isk->ack_seq, SYN_N, FIN_N, ACK_NOW); /* 如果不是则回复期待的seq */
            break;
        }  
        else
        {
            isk->ack_seq = tcp_get_ackseq(skb_tmp); /* 如果是则更新期待的seq并发送之  */
            ret = tcp_send_ack(isk, tcph, isk->send_seq, isk->ack_seq, SYN_N, FIN_N, ACK_NOMAL);
            if (ret < 0)
                break;
        }
        list = list->next; /* 循环判断是否是上面注释当中的2、3种情况，如果是则发送所有可回复ack  */
    } 
    exit_critical();

    if (isk->ack_backlog >= isk->max_ack_backlog)
    {
        enter_critical();
        while (!list_empty(&isk->ack_queue)) 
            tcp_timer_entry(isk);
        
        exit_critical();
    }

    return ret;
}

int tcp_data(struct i_socket *isk, struct tcphdr *tcph, struct sk_buff *skb)
{
    return tcp_seg_sort(isk, skb);
}

int tcp_fin_handler(struct i_socket *isk, struct tcphdr *tcph)
{
    if (!isk || !tcph)
        return -ENOENT;

    tcp_send_ack(isk, tcph, isk->received_ack, ntohl(tcph->seq) + 1, SYN_N, FIN_N, ACK_NOW);

    switch (isk->status)
    {
    case ESTABLISHED:
        isk->status = CLOSE_WAIT;
        break;
    case FIN_WAIT_1://xxxxxxxxxxxxxxxxxxxxxxxx
    case FIN_WAIT_2:
        isk->retries = TCP_RETRY_TIMES;
        isk->status = TIME_WAIT;
        isk->timeout = OS_Get_Kernel_Ticks();
        if (mod_timer(&isk->timer, 20) < 0)
            free_isock(isk);
        break;
    case CLOSE_WAIT:
    case TIME_WAIT:
        break;
    default: 
        break;
    }
    
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
    struct iphdr *iph;
    
    iph = get_iph(skb);
    tcph = get_tcph(skb);
    
    length = (tcph->doff * 4) - sizeof (struct tcphdr);
    ptr = (unsigned char *)(tcph + 1);

    printk("source:  %d\n", ntohs(tcph->source));
    printk("dest:    %d\n", ntohs(tcph->dest));
    printk("doff:    %d\n", tcph->doff);
    printk("seq:     %x\n", ntohl(tcph->seq));
    printk("next_seq %x\n", ntohl(tcph->seq) + (ntohs(iph->tot_len) - SIZEOF_IPHDR - tcph->doff * 4));
    printk("ack_seq: %x\n", ntohl(tcph->ack_seq));
    printk("syn:     %d\n", tcph->syn);
    printk("ack:     %d\n", tcph->ack);
    printk("psh:     %d\n", tcph->psh);
    printk("window:  %d\n", ntohs(tcph->window));
    printk("check:   %x\n", ntohs(tcph->check));
    printk("datalen: %d\n", ntohs(iph->tot_len) - SIZEOF_IPHDR - tcph->doff * 4);
    
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
    int ret = 0;
	struct tcphdr *tcph;
	struct i_socket *isk;

	if (!skb)
		return 0;
    
	tcph = get_tcph(skb);
	isk = tcp_find_sock(skb, tcph);

	if (!isk)
	{
		free_skbuff(skb);
		return -ENOTSOCK;///xxxxxxxxxxxxxxxxxxxxxxxx
	}

	switch (isk->status)
	{
	case LAST_ACK:
           ret = tcp_ack_handler(isk, tcph);
        break;
	case ESTABLISHED:
	case CLOSE_WAIT:
	case FIN_WAIT_1:
	case FIN_WAIT_2:
	case TIME_WAIT:
		if (tcph->rst || tcph->syn)
		{
			isk->errno = -ECONNRESET;
			isk->status = CLOSED;
            break;
		}
		
		if (tcph->ack)
		{
            ret = tcp_ack_handler(isk, tcph);
            if (ret < 0)
                break;
		}
		
		if (tcph->urg)
		{
            ret = tcp_urg_handler();
            if (ret < 0)
                break;
		}
		
		if (tcph->fin)
		{
            ret = tcp_fin_handler(isk, tcph);
            if (ret < 0)
		    {
                isk->errno  = -ECONNRESET;
                isk->status = CLOSED;
                break;
            }
            else
            {
                isk->retries = TCP_RETRY_TIMES;
                isk->timeout = OS_Get_Kernel_Ticks();
                tcp_send_ack(isk, tcph, isk->received_ack, ntohl(tcph->seq) + 1, SYN_N, FIN_Y, ACK_NOW);
                isk->status = LAST_ACK;
            }
            break;
		}

        if (tcp_get_data_lenght(skb) > 0)
	        return tcp_data(isk, tcph, skb);
		break;
	case SYN_SENT:
		if (tcph->rst)
		{
			isk->errno = -ECONNRESET;
			isk->status = CLOSED;
            break;
		}
		
        if (!tcph->syn || !tcph->ack)
            break;

	    ret = tcp_ack_handler(isk, tcph);
        if (ret < 0)
		{
			tcp_reset(isk, tcph);
            break;
		}

        isk->send_seq = htonl(tcph->ack_seq);
        isk->ack_seq  = htonl(tcph->seq) + 1;
        tcp_send_ack(isk, tcph, isk->send_seq, isk->ack_seq, SYN_N, FIN_N, ACK_NOW);
		tcp_set_options(isk, skb);
        complete_establish(isk);
		break;
	case SYN_RCVD:
		ret = tcp_ack_handler(isk, tcph);
        if (ret < 0 || tcph->syn)
        {
			tcp_reset(isk, tcph);
            break;
		}
		
		isk->status = ESTABLISHED;
        complete_establish(isk);
		break;
	case CLOSED:
		break;
	case LISTEN:
		if (tcph->rst || tcph->ack)
            break;

		if (tcph->syn)
			ret = tcp_connect_request_handler(skb, isk, tcph);
        
        if (ret == 0)
            isk->status = SYN_RCVD;
        break;
    }

    free_skbuff(skb);
	return ret;
}

int tcp_process(struct sk_buff *skb)
{
    return tcp_received_data_handler(skb);
}

int tcp_connect(struct i_socket *isk, struct sockaddr_in *sin, int addr_len)
{
	int ret;
    struct ip_addr *dest;
    struct net_device *ndev;

    isk->remote_ip   = ntohl(sin->sin_addr.addr);
    isk->remote_port = ntohs(sin->sin_port);
    isk->local_port  = tcp_get_an_unused_port();

    dest = &sin->sin_addr;
    ndev = ip_route(dest);
    if (!ndev)
        return -ENETUNREACH;
    isk->local_ip = ndev->ip;

    isk->retries = TCP_RETRY_TIMES;
	isk->timer.expires = 10;
	isk->timer.data    = isk;
	isk->timer.function = tcp_timer_entry;

	ret = tcp_send_syn(isk);
	if (ret < 0)
		return ret;
	
	isk->status = SYN_SENT;
    isk->timeout = OS_Get_Kernel_Ticks();
	ret = add_timer(&isk->timer);
	if (ret < 0)
		return ret;
	
    list_add(&isk->list, &active_queue);
	wait_event(&isk->wq, isk->status != SYN_SENT);
	
    if (isk->errno)
        list_del(&isk->list);

	return isk->errno;
}

void tcp_close(struct i_socket *isk, int timeout)
{
    switch (isk->status)
    {
    case ESTABLISHED:
    case SYN_SENT:
    case SYN_RCVD:
        if (tcp_send_fin(isk, isk->send_seq, isk->ack_seq) < 0)
        {
            free_isock(isk);
            return;
        }
        isk->timeout = OS_Get_Kernel_Ticks();
        isk->status = FIN_WAIT_1;
        break;
    case CLOSED:
        free_isock(isk);
        return;
    default:
        break;
    }
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


int	tcp_recv(struct i_socket *isk, char *ubuf, int len, int nonblock, unsigned flags)
{
    char *dest, *src;
    struct sk_buff *skb;
    int ubuf_len_remain = len;
    unsigned short skb_data_remain;
    struct list_head *list;
    
    if (!isk)
        return -EBADF;

    if (!ubuf)
        return -EFAULT;

    if (nonblock && !list_empty(&isk->recv_data_head))
        return -EAGAIN;

    //wait_event(&isk->wq, !list_empty(&isk->recv_data_head));
    wait_event(&isk->wq, isk->recv_data_len);

    if (isk->errno)
        return isk->errno;

    /*
     * 以下提到的所有 "数据" 均指tcp的数据部分,即tcp头及选项(如果有的话)后面的数据
     * 每个tcp skb最多容纳isk->mss(一般为1460)字节数据
     * 每个链接接收到的所有有效数据以skb形式挂在isk->recv_data_head
     * isk->read_ptr表示某个skb已经被读取的字节数,假如对方发了1024个
     * 字节,某一次应用只读取了32字节,这时read_ptr即为32,应用程序下一
     * 次读会从第32个字节开始读取数据,直至这个skb的1024字节全被读取
     * 之后即释放这个sbk,isk->read_ptr重赋值为0,表示从下一个skb的数据
     * 0地址开始读
     */
    
    enter_critical();
    src = ubuf;
    list = isk->recv_data_head.next;
    while (list != &isk->recv_data_head)
    {
        skb = list_entry(list, struct sk_buff, list);
        list = list->next;

        dest = tcp_get_data_off(skb) + isk->read_ptr;

        skb_data_remain = tcp_get_data_lenght(skb) - isk->read_ptr;
        if (skb_data_remain <= ubuf_len_remain)
        {
            memcpy(src, dest, skb_data_remain);
            ubuf_len_remain -= skb_data_remain;
            isk->read_ptr = 0;
            src += skb_data_remain;
            free_skbuff(skb);
        }
        else
        {
            memcpy(src, dest, ubuf_len_remain);
            isk->read_ptr += ubuf_len_remain;
            ubuf_len_remain = 0;
            break;
        }
    }
    exit_critical();

    isk->recv_data_len -= (len - ubuf_len_remain);
	return len - ubuf_len_remain;
}

int tcp_listen(struct i_socket *isk, int backlog)
{
    if (isk->status != CLOSED)	
        return -EINVAL;

    isk->status = LISTEN;
    isk->backlog = backlog;
    
    /* 此isk一直存在于原始的struct socket中,无需从listen_queue中查找,
     * 所以不需放入listen_queue队列中,此队列用于存放调用了accept之后
     * 的new_isk,见tcp_accept函数
     */
    
    //list_add_tail(&isk->list, &listen_queue);
    
    return 0;
}

int tcp_send(struct i_socket *isk, char *buf, int len, int nonblock, unsigned flags)
{
    if (!isk)
        return -ENOTSOCK;
    if(!buf)
        return -EINVAL;
    if (!len)
        return 0;

    return tcp_do_segment(isk, buf, len);
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

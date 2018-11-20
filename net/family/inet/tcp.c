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
#include "head.h"

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

    iph = skb->data_buf + OFFSET_IPHDR;

    if (tcph->syn && tcph->ack)
    {
        if (list_empty(&active_queue))
            return NULL;
        list_for_each(list, &active_queue)
        {
            isk = list_entry(list, struct i_socket, list);
            if ((isk->remote_ip == ntohl(iph->saddr)) && \
                (isk->remote_port == ntohs(tcph->source)));
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
            if (isk->local_port == ntohs(tcph->dest));
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

}

int tcp_send_syn(struct i_socket *isk)
{

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

void tcp_send_ack(struct i_socket *isk, struct tcphdr *tcph, unsigned int seq, unsigned char is_syn)
{

}

void set_tcp_options()
{

}

int tcp_connect_request_handler(struct sk_buff *skb, struct i_socket *isk, struct tcphdr *tcph)
{
    int err;
    struct iphdr *iph;
    struct i_socket *new_isk; 

    //if (!skb || !isk || !tcph)
    //   return -XXX;

    iph = skb->data_buf + OFFSET_IPHDR;

    new_isk = alloc_isocket();
    if (!new_isk)
        return -EAGAIN;

    memcpy(new_isk, isk, sizeof (struct i_socket));

    new_isk->remote_ip   = ntohl(iph->saddr);
    new_isk->remote_port = ntohs(tcph->source);

    tcp_send_ack(new_isk, tcph, tcph->seq + 1, 1);
    new_isk->status = SYN_RCVD;
    list_add_tail(&new_isk->list, &active_queue);
    
    return 0;
}

void complete_establish(struct i_socket *isk)
{
    struct socket *socket;

    if (!isk || isk->socket)
        return;

    socket = isk->socket;
    isk->socket = NULL;

    socket->state = ESTABLISHED;
    wake_up(&socket->wq);
    //wake_up
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

int tcp_received_data_handler(struct sk_buff *skb)
{
	struct tcphdr *tcph;
	struct i_socket *isk;

	if (!skb)
		return 0;
	
	tcph = (struct tcphdr *)(skb->data_buf + OFFSET_TCPHDR);
	isk = find_sock(skb, tcph);

	if (!isk)
	{
		free_skbuff(skb);
		return 0;///xxxxxxxxxxxxxxxxxxxxxxxx
	}
	//list_add_tail(&skb->list, &isk->back_log);

	switch (isk->status)
	{
	case LAST_ACK:
		isk->errno 	= -ECONNRESET;
		isk->status 	= CLOSED;
		free_skbuff(skb);
		release_sock(isk);
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
			release_sock(isk);
			return 0;
		}
		
		if (tcph->syn)
		{
			isk->errno = -ECONNRESET;
			isk->status = CLOSED;
			free_skbuff(skb);
			release_sock(isk);
			return 0;
		}

		if (tcph->ack && (tcp_ack_handler(isk, tcph) < 0))
		{
			free_skbuff(skb);
			release_sock(isk);
			return 0;
		}
		
		if (tcph->urg && (tcp_urg_handler() < 0))
		{
			free_skbuff(skb);
			release_sock(isk);
			return 0;
		}
		
		if (tcph->fin && (tcp_fin_handler() < 0))
		{
			free_skbuff(skb);
			release_sock(isk);
			return 0;
		}

		if (tcp_data(isk, tcph, skb) < 0)
		{
			free_skbuff(skb);
			release_sock(isk);
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
		tcp_send_ack(isk, tcph, tcph->seq + 1, 0);
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
		if (!tcph->syn)
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
			return tcp_connect_request_handler(isk, tcph, skb);
		else
		{
			free_skbuff(skb);
			return 0;
		}
		break;
	}

	return 0;
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
	struct sk_buff *skb;

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

int	tcp_read(struct i_socket *isk, unsigned char *to, int len, int nonblock, unsigned flags)
{

}

int	tcp_write(struct i_socket *isk, unsigned char *to,  int len, int nonblock, unsigned flags)
{

}

int tcp_sendto(struct i_socket *isk, unsigned char *buf, int len, int noblock,
			  unsigned flags, struct sockaddr_in *usin, int addr_len)
{

}

int	tcp_recvfrom(struct i_socket *isk, unsigned char *buf, int len, int noblock,
				unsigned flags, struct sockaddr_in *usin, int *addr_len)
{

}

int tcp_connect(struct i_socket *isk, struct sockaddr_in *sin, int addr_len)
{

}

int tcp_bind(struct i_socket *isk, struct sockaddr_in *sin, int addrlen)
{
	
}


int	tcp_recv(struct i_socket *isk, char *buf, int len, int nonblock, unsigned flags)
{

}

int tcp_listen(struct i_socket *isk, int backlog)
{
    if (isk->status != CLOSED)	
        return -EINVAL;

    isk->status = LISTEN;
    isk->backlog = backlog;
    list_add_tail(&isk->list, &listen_queue);
    
    return 0;
}

int tcp_send(struct i_socket *isk, unsigned char *buf, int len, int nonblock, unsigned flags)
{
	
}

struct i_proto_opt tcp_opt[] =
{
	tcp_close,
	tcp_read,
	tcp_write,
	tcp_sendto,
	tcp_recvfrom,
	tcp_connect,
	tcp_bind,
	NULL,
	tcp_listen,
	tcp_send,
	tcp_recv
};

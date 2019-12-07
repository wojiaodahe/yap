#ifndef __I_SOCKET_H__
#define __I_SOCKET_H__

#include "icmp.h"
#include "arp.h"
#include "eth.h"
#include "wait.h"
#include "timer.h"
#include "tcp.h"

/* Structure describing an Internet (IP) socket address. */
#define __SOCK_SIZE__	16		/* sizeof(struct sockaddr)	*/

struct ip_addr
{
	unsigned int addr;
}__attribute__((packed));

struct sockaddr_in
{
  short int				sin_family;	/* Address family		*/
  unsigned short int	sin_port;	/* Port number			*/
  struct ip_addr		sin_addr;	/* Internet address		*/

  /* Pad to size of `struct sockaddr'. */
  unsigned char		    __pad[6];
}__attribute__((packed));


struct i_socket_priv
{
	int priv;
};

struct i_socket
{
	int 				 	 prot_type;                   /* inet 族协议类型 tcp or udp or raw or ... */
	void 				  	*prot;                       /* inet 族协议控制块 tcp_pcb udp_pcb raw_pcb ... */
	struct i_proto_opt 	  	*i_prot_opt;  				  /* inet 族每种协议的操作接口 */
	struct i_socket_priv 	*priv;
	struct socket		  	*socket;
    unsigned int            recv_data_len;
    unsigned short          read_ptr;
	struct list_head      	recv_data_head;
	struct list_head     	send_data_head;
    int                     backlog;
	int 				  	errno;
	wait_queue_t		  	wq;
    unsigned char           sack;
	unsigned short 			mss;
	unsigned short        	mtu;
    unsigned int            rto;
    unsigned int            rtt;
	volatile unsigned short window;
    unsigned int            window_shift;
    struct sliding_window   send_window;
	volatile unsigned int 	flags;
	volatile unsigned int 	status;
    unsigned int            local_ip;
    unsigned int            remote_ip;
	unsigned short int    	local_port;
	unsigned short int 	 	remote_port;
	unsigned short int 	  	ssthresh;
	unsigned short int		timeout;
	unsigned short int		retries;
	struct timer_list     	timer;

    unsigned char           max_ack_backlog;
    unsigned char           ack_backlog;
    struct list_head        ack_queue;
   
    volatile unsigned int   received_ack;
    volatile unsigned int   ack_seq;
    volatile unsigned int   send_seq;

    struct list_head        list;
};


struct i_proto_opt
{

    int proto;
	void (*close)(struct i_socket *isk, int timeout);
	int	 (*read)(struct i_socket *isk, char *to, int len, int nonblock, unsigned flags);
	int	 (*write)(struct i_socket *isk, char *to,  int len, int nonblock, unsigned flags);
	int	 (*sendto)(struct i_socket *isk, char *buf, int len, int noblock,
				  unsigned flags, struct sockaddr_in *usin, int addr_len);
	int	 (*recvfrom)(struct i_socket *isk, char *buf, int len, int noblock,
					unsigned int flags, struct sockaddr_in *usin, int *addr_len);
	int	 (*connect)(struct i_socket *isk, struct sockaddr_in *usin, int addr_len);
	int  (*bind)(struct i_socket *isk, struct sockaddr_in *usin, int addrlen);
	struct i_socket *(*accept)(struct i_socket *isk, int flags);
	int  (*listen)(struct i_socket *isk, int backlog);
	int	 (*send)(struct i_socket *isk, char *to, int len, int nonblock, unsigned flags);
	int	 (*recv)(struct i_socket *isk, char *to, int len, int nonblock, unsigned flags);
    void (*proto_new_sock)(struct i_socket *isk);
};

#define PACKET_RECVED	(1 << 0)

#define SIZEOF_ETHHDR	(sizeof (struct ethhdr))
#define SIZEOF_IPHDR	(sizeof (struct iphdr))
#define SIZEOF_ICMPHDR	(sizeof (struct icmphdr))
#define SIZEOF_UDPHDR	(sizeof (struct udphdr))
#define SIZEOF_TCPHDR	(sizeof (struct tcphdr))

#define OFFSET_IPHDR	(SIZEOF_ETHHDR)
#define OFFSET_IPDATA   (SIZEOF_ETHHDR + SIZEOF_IPHDR)
#define OFFSET_ICMPHDR	(SIZEOF_ETHHDR + SIZEOF_IPHDR)
#define OFFSET_UDPHDR	(SIZEOF_ETHHDR + SIZEOF_IPHDR)
#define OFFSET_UDPDATA	(SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_UDPHDR)
#define OFFSET_TCPHDR	(SIZEOF_ETHHDR + SIZEOF_IPHDR)
#define OFFSET_TCPDATA  (SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_TCPHDR)
#define OFFSET_TCPOPT   (SIZEOF_ETHHDR + SIZEOF_IPHDR + SIZEOF_TCPHDR)

#define get_iph(skb)    ((struct iphdr *)(skb->data_buf + OFFSET_IPHDR))
#define get_tcph(skb)   ((struct tcphdr *)(skb->data_buf + OFFSET_TCPHDR))

#define INET_PROTO_UDP	0x11
#define INET_PROTO_TCP  0x06

#define  MAX_INET_PROTO 3 //tcp udp raw

extern struct i_socket *alloc_isocket(void);
extern void release_sock(struct i_socket *isk);
extern int inet_register_proto(struct i_proto_opt *opt);
extern void free_isock(struct i_socket *isk);

#endif


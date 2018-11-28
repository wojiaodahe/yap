#ifndef INCLUDE_IP_H_
#define INCLUDE_IP_H_

#include "inet_socket.h"
#include "socket.h"

struct iphdr
{
  unsigned char		version;
  unsigned char		tos;
  unsigned short	tot_len;
  unsigned short	id;
  unsigned short	frag_off;
  unsigned char		ttl;
  unsigned char		protocol;
  unsigned short	check;
  unsigned int		saddr;
  unsigned int		daddr;
}__attribute__((packed));

struct ipfrag
{
	unsigned short id;
	struct sk_buff *skb;
	struct list_head head;
	unsigned short current_len;
	unsigned short original_len;
    unsigned long  timeout;
};


#define PROTO_TCP	0x06
#define PROTO_UDP	0x11
#define PROTO_RAW
#define PROTO_ICMP	0x01


#define IP_CE		    0x8000		/* Flag: "Congestion"		*/
#define IP_DF		    0x4000		/* Flag: "Don't Fragment"	*/
#define IP_MF		    0x2000		/* Flag: "More Fragments"	*/
#define IP_FRAG_MASK    (~(IP_CE | IP_DF | IP_MF))
#define IP_OFFSET

#define IP_TICKS            10
#define IP_FRAG_LIFETIME    100
#define IP_FRAG_LIST_HEAD_NUM   32
#define IP_FRAG_LIST_NO_USED    0

extern int ip_send(struct sk_buff *skb, struct ip_addr *dest, unsigned char proto);
extern struct net_device *ip_route(struct ip_addr *dest);
extern int ip_recv(struct sk_buff *skb);
extern void ip_init(void);

#endif


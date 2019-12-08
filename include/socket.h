#ifndef __KN_SOCKET_H__
#define __KN_SOCKET_H__
#include "wait.h"

#define SKB_USE_SKB_POOL	1

#define MTU	1518
#define MAX_SKB_NUM	5000

#define SKB_NO_USE	0
#define SKB_USED	1

struct sk_buff
{
	char use_flag;
	char *data_buf;
	unsigned short data_len;
	unsigned int timeout;
	struct net_device	*ndev;
	struct list_head list;
};

#define MAX_SOCKETS		128		
#define MAX_PROTO		16		

#define SOCK_INODE(S)	((S)->inode)

/* Socket types. */
#define SOCK_STREAM		1		/* stream (connection) socket	*/
#define SOCK_DGRAM		2		/* datagram (conn.less) socket	*/
#define SOCK_RAW		3		/* raw socket			*/
#define SOCK_RDM		4		/* reliably-delivered message	*/
#define SOCK_SEQPACKET	5   	/* sequential packet socket	*/
#define SOCK_PACKET		10		/* linux specific way of	*/

#define SO_ACCEPTCON	(1 << 16)		/* performed a listen		*/


struct sockaddr 
{
	unsigned short	sa_family;	    /* address family, AF_xxx	*/
	char			sa_data[14];	/* 14 bytes of protocol address	*/
};

typedef enum 
{
	SS_FREE = 0,				/* not allocated		*/
	SS_UNCONNECTED,			/* unconnected to any socket	*/
	SS_CONNECTING,			/* in process of connecting	*/
	SS_CONNECTED,				/* connected to socket		*/
	SS_DISCONNECTING			/* in process of disconnecting	*/
} socket_state;

#define AF_UNSPEC	0
#define AF_UNIX		1
#define AF_INET		2
#define AF_AX25		3
#define AF_IPX		4

struct socket 
{
	int fd;
	short int			type;		/* SOCK_STREAM, ...		*/
	socket_state		state;
	long				flags;
	struct family_ops	*ops;		/* protocols do most everything	*/
	void				*data;	    /* protocol data		*/
	struct socket		*conn;		/* server socket connected to	*/
	struct socket		*iconn;		/* incomplete client conn.s	*/
	struct socket		*next;
	struct inode		*inode;
	wait_queue_t 		wq;
	void 				*rw_buf;
};

struct family_ops
{
	int	family;
	int	(*socket)	(struct socket *sock, int protocol);
	int	(*dup)		(struct socket *newsock, struct socket *oldsock);
	int	(*close)	(struct socket *sock, struct socket *peer);
	int	(*bind)		(struct socket *sock, struct sockaddr *umyaddr, int sockaddr_len);
	int	(*connect)	(struct socket *sock, struct sockaddr *uservaddr, int sockaddr_len, int flags);
	int	(*accept)	(struct socket *sock, struct socket *newsock, int *addrlen);
	int	(*read)		(struct socket *sock, char *ubuf, int size, int nonblock);
	int	(*write)	(struct socket *sock, char *ubuf, int size, int nonblock);
	int	(*ioctl)	(struct socket *sock, unsigned int cmd, unsigned long arg);
	int	(*listen)	(struct socket *sock, int len);
	int	(*send)		(struct socket *sock, void *buff, int len, int nonblock, unsigned flags);
	int	(*recv)		(struct socket *sock, void *buff, int len, int nonblock, unsigned flags);
	int	(*sendto)	(struct socket *sock, void *buff, int len, int nonblock, unsigned flags, struct sockaddr *, int addr_len);
	int	(*recvfrom)	(struct socket *sock, void *buff, int len, int nonblock, unsigned flags, struct sockaddr *, int *addr_len);
};

extern struct sk_buff *alloc_skbuff(unsigned int short data_len);
extern void free_skbuff(struct sk_buff *skb);
extern void free_all_skb(struct list_head *head);
extern int socket_register(int family, struct family_ops *ops);
extern int sys_socket(int family, int type, int protocol);
extern int sys_bind(int fd, struct sockaddr *umyaddr, int addrlen);
extern int sys_listen(int fd, int backlog);
extern int sys_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen);
extern int sys_connect(int fd, struct sockaddr *uservaddr, int addrlen);
extern int sys_send(int fd, void * buff, int len, unsigned flags);
extern int sys_sendto(int fd, void * buff, int len, unsigned flags, struct sockaddr *addr, int addr_len);
extern int sys_recv(int fd, void * buff, int len, unsigned flags);
extern int sys_recvfrom(int fd, void * buff, int len, unsigned flags, struct sockaddr *addr, int *addr_len);
extern int socket_register(int family, struct family_ops *ops);
extern int socket_init(void);

#endif


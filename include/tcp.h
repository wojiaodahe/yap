#ifndef _TCP_H__
#define _TCP_H__
#include "list.h"

#define TCY_SYN_RETRIES		5

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_ECE 0x40
#define TCP_CWR 0x80

enum
{
  CLOSED      = 0,
  LISTEN      = 1,
  SYN_SENT    = 2,
  SYN_RCVD    = 3,
  ESTABLISHED = 4,
  FIN_WAIT_1  = 5,
  FIN_WAIT_2  = 6,
  CLOSE_WAIT  = 7,
  CLOSING     = 8,
  LAST_ACK    = 9,
  TIME_WAIT   = 10
};

struct sliding_window
{
    volatile unsigned int tot_size;
    volatile unsigned int used_size;

    struct list_head wait;
    struct list_head ready;
    struct list_head wait_ack;
};

struct tcp_seg
{
	void *data;
    unsigned int send_seq;
    unsigned short len;
	unsigned short retries;
	
    unsigned int expected_ack;

    struct i_socket *isk;
	struct list_head list;
};

struct tcphdr
{
	unsigned short source;
	unsigned short dest;
	unsigned long  seq;
	unsigned long  ack_seq;
	unsigned short res1:4,
				   doff:4,
				   fin:1,
				   syn:1,
				   rst:1,
				   psh:1,
				   ack:1,
				   urg:1,
				   res2:2;

	unsigned short window;
	unsigned short check;
	unsigned short urg_ptr;
}__attribute__((packed));

#define MAX_TCP_HEADER_SIZE 60

/*
 *	TCP option
 */
 
#define TCPOPT_NOP		        1	/* Padding */
#define TCPOPT_EOL		        0	/* End of options */
#define TCPOPT_MSS		        2	/* Segment size negotiating */
#define TCPOPT_WINDOW		    3	/* Window scaling */
#define TCPOPT_SACK_PERM        4       /* SACK Permitted */
#define TCPOPT_SACK             5       /* SACK Block */
#define TCPOPT_TIMESTAMP	    8	/* Better RTT estimations/PAWS */
#define TCPOPT_MD5SIG		    19	/* MD5 Signature (RFC2385) */

/*
 *     TCP option lengths
 */

#define TCPOLEN_MSS            4
#define TCPOLEN_WINDOW         3
#define TCPOLEN_SACK_PERM      2
#define TCPOLEN_TIMESTAMP      10
#define TCPOLEN_MD5SIG         18


#define TS_Y        1
#define TS_N        0
#define ACK_Y       1
#define ACK_N       0
#define FIN_Y       1
#define FIN_N       0
#define SYN_Y       1
#define SYN_N       0
#define OPT_Y       1
#define OPT_N       0
#define MSS_Y       1
#define MSS_N       0
#define SACK_Y      1
#define SACK_N      0
#define WSCALE_Y    1
#define WSCALE_N    0

#define TCPOPT_WSCALE_VALUE     7

#define ACK_NOW     1
#define ACK_NOMAL   0


/* timeout  */
/* 对方法了syn 但在这个时间之内没有再发ack过来,则取消此次连接 */
#define SYN_RCVD_HOLDING_TIME   1000  
#define TCP_FIN_WAIT_TIME       500

#define TCP_RETRY_TIMES         10
#define TCP_SEG_SEND_RETRIES    10

#define TCP_SEG_TIMEOUT_FLAG    (1 << 0)

extern int tcp_process(struct sk_buff *skb);
extern int tcp_init(void);
#endif

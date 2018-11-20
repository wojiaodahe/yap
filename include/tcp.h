#ifndef _TCP_H__
#define _TCP_H__

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
	void *head;
	void *tail;
	unsigned int segs;
};

struct tcp_seg
{
	void *data;
    unsigned short len;
	unsigned short retry_times;
	
    unsigned int expected_ack;

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
	unsigned short chedk;
	unsigned short urg_ptr;
}__attribute__((packed));

#endif

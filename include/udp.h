/*
 * udp.h
 *
 *  Created on: 2018Äê7ÔÂ3ÈÕ
 *      Author: crane
 */

#ifndef INCLUDE_UDP_H_
#define INCLUDE_UDP_H_

#define UDP_IS_USED		(1 << 0)
#define UDP_IS_UNUSED	(0)

struct udp
{
	unsigned char flags;
	struct i_socket *isk;
	unsigned short int local_port;
	unsigned short int remote_port;

	struct list_head link;
};

struct udphdr
{
	unsigned short	source;
	unsigned short	dest;
	unsigned short	len;
	unsigned short	check;
}__attribute__((packed));



struct udp_pseudo_hdr
{
	int sip;
	int dip;
	char mbz;
	char proto;
	unsigned short len;
}__attribute__((packed));

#endif /* INCLUDE_UDP_H_ */

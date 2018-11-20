/*
 * ip.h
 *
 *  Created on: 2018Äê6ÔÂ27ÈÕ
 *      Author: crane
 */

#ifndef INCLUDE_IP_H_
#define INCLUDE_IP_H_

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

#define PROTO_TCP	0x06
#define PROTO_UDP	0x11
#define PROTO_RAW
#define PROTO_ICMP	0x01


#define IP_CE		0x8000		/* Flag: "Congestion"		*/
#define IP_DF		0x4000		/* Flag: "Don't Fragment"	*/
#define IP_MF		0x2000		/* Flag: "More Fragments"	*/
#define IP_OFFSET

#endif /* INCLUDE_IP_H_ */

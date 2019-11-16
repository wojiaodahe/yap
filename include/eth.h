#ifndef INCLUDE_ETH_H_
#define INCLUDE_ETH_H_

#include "socket.h"

#define ETH_P_LOOP	0x0060		/* Ethernet Loopback packet	*/
#define ETH_P_ECHO	0x0200		/* Ethernet Echo packet		*/
#define ETH_P_PUP	0x0400		/* Xerox PUP packet		*/
#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
#define ETH_P_ARP	0x0806		/* Address Resolution packet	*/
#define ETH_P_RARP  0x8035		/* Reverse Addr Res packet	*/
#define ETH_P_X25	0x0805		/* CCITT X.25			*/
#define ETH_P_IPX	0x8137		/* IPX over DIX			*/
#define ETH_P_802_3	0x0001		/* Dummy type for 802.3 frames  */
#define ETH_P_AX25	0x0002		/* Dummy protocol id for AX.25  */
#define ETH_P_ALL	0x0003		/* Every packet (be careful!!!) */

/* This is an Ethernet frame header. */
struct ethhdr
{
	unsigned char	h_dest[6];		/* destination eth addr	*/
	unsigned char	h_source[6];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
}__attribute__((packed));

extern int eth_recv(struct sk_buff *skb);


#endif /* INCLUDE_ETH_H_ */

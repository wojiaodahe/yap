/*
 * icmp.h
 *
 *  Created on: 2018Äê6ÔÂ27ÈÕ
 *      Author: crane
 */

#ifndef INCLUDE_ICMP_H_
#define INCLUDE_ICMP_H_

#define ICMP_ER 	0      /* echo reply */
#define ICMP_DUR 	3     /* destination unreachable */
#define ICMP_SQ 	4      /* source quench */
#define ICMP_RD 	5      /* redirect */
#define ICMP_ECHO 	8    /* echo */
#define ICMP_TE 	11     /* time exceeded */
#define ICMP_PP 	12     /* parameter problem */
#define ICMP_TS 	13     /* timestamp */
#define ICMP_TSR 	14    /* timestamp reply */
#define ICMP_IRQ 	15    /* information request */
#define ICMP_IR		16     /* information reply */

struct icmphdr
{
	unsigned char  type;
	unsigned char  code;
	unsigned short chksum;
	unsigned short id;
	unsigned short seqno;
	//unsigned int   timestamp;
}__attribute__((packed));

#endif /* INCLUDE_ICMP_H_ */



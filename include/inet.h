#ifndef __INET_H__
#define __INET_H__

struct inet_pseudo_hdr
{
	int sip;
	int dip;
	char mbz;
	char proto;
	unsigned short len;
}__attribute__((packed));



extern unsigned short htons(unsigned short h);
extern unsigned int htonl(unsigned int h);
extern unsigned int ntohl(unsigned long int n);
extern unsigned short ntohs(unsigned short int n);
extern unsigned short inet_chksum(void *buffer, unsigned short size);
extern int net_core_init(void);
extern short int inet_check(struct ip_addr *sip, struct ip_addr *dip, unsigned char *buffer, unsigned short size, int proto);
extern unsigned short __inet_chksum(void *buffer, unsigned short size);


extern unsigned short htons(unsigned short h);
extern unsigned int htonl(unsigned int h);
extern unsigned int ntohl(unsigned long int n);
extern unsigned short ntohs(unsigned short int n);

#endif



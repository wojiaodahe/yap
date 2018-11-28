#ifndef __INET_H__
#define __INET_H__


extern unsigned short htons(unsigned short h);
extern unsigned int htonl(unsigned int h);
extern unsigned int ntohl(unsigned long int n);
extern unsigned short ntohs(unsigned short int n);
extern unsigned short inet_chksum(void *buffer, unsigned short size);
extern void net_core_init(void);

#endif



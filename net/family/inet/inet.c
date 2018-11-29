#include "inet_socket.h"
#include "netdevice.h"
#include "arp.h"
#include "inet.h"

#define BigLittleSwap16(n) ((((unsigned short)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))

#define BigLittleSwap32(A) ((((unsigned int)(A) & 0xff000000) >> 24) | \
						    (((unsigned int)(A) & 0x00ff0000) >> 8)  | \
						    (((unsigned int)(A) & 0x0000ff00) << 8)  | \
						    (((unsigned int)(A) & 0x000000ff) << 24))

int checkCPUendian()
{
       union
	   {
              unsigned long i;
              unsigned char s[4];
       }c;
       c.i = 0x12345678;

       return (0x12 == c.s[0]);
}

unsigned short htons(unsigned short h)
{
	return checkCPUendian() ? h : BigLittleSwap16(h);
}

unsigned int htonl(unsigned int h)
{
	return checkCPUendian() ? h : BigLittleSwap32(h);
}

unsigned int ntohl(unsigned long int n)
{
    return checkCPUendian() ? n : BigLittleSwap32(n);
}

unsigned short ntohs(unsigned short int n)
{
	return checkCPUendian() ? n : BigLittleSwap16(n);
}

#if 0
unsigned short inet_chksum(void *dataptr, unsigned short len)
{
  unsigned int acc;
  unsigned short src;
  unsigned char *octetptr;

  acc = 0;
  /* dataptr may be at odd or even addresses */
  octetptr = (unsigned char*)dataptr;
  while (len > 1) {
    /* declare first octet as most significant
       thus assume network order, ignoring host order */
    src = (*octetptr) << 8;
    octetptr++;
    /* declare second octet as least significant */
    src |= (*octetptr);
    octetptr++;
    acc += src;
    len -= 2;
  }
  if (len > 0) {
    /* accumulate remaining octet */
    src = (*octetptr) << 8;
    acc += src;
  }
  /* add deferred carry bits */
  acc = (acc >> 16) + (acc & 0x0000ffffUL);
  if ((acc & 0xffff0000UL) != 0) {
    acc = (acc >> 16) + (acc & 0x0000ffffUL);
  }
  /* This maybe a little confusing: reorder sum using htons()
     instead of ntohs() since it has a little less call overhead.
     The caller must invert bits for Internet sum ! */
  return htons((unsigned short)acc);
}
#else
unsigned short __inet_chksum(void *buffer, unsigned short size)
{
	__packed unsigned short *tmp;
	unsigned long cksum=0;

	tmp = (unsigned short *)buffer;
	while(size >1)
	{
		cksum += *tmp++;
		size -= sizeof(unsigned short);
	}
	if(size) 
		cksum += *tmp;
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (unsigned short)(~cksum);
}

short int inet_check(struct ip_addr *sip, struct ip_addr *dip, unsigned char *buffer, unsigned short size, int proto)
{
    struct inet_pseudo_hdr *phead;
    short int check_sum = 0;         //校验和字段置零；原来程序中定义为unsigned long

    phead = (struct inet_pseudo_hdr *)(buffer - sizeof (struct inet_pseudo_hdr));      //缓存数组转换成结构体指针
    phead->sip   = sip->addr;
    phead->dip   = dip->addr;
    phead->mbz   = 0;
    phead->proto = proto;          
    phead->len   = htons(size);

    check_sum = __inet_chksum(phead, sizeof (struct inet_pseudo_hdr) + size);

    return check_sum;
}

#endif

int net_core_init(void)
{
    int ret;

    net_device_core_init();
    ip_init();
    ret =  arp_init();
    
    return ret;
}

/*
 * inet.c
 *
 *  Created on: 2018Äê6ÔÂ29ÈÕ
 *      Author: crane
 */

#include "inet_socket.h"

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

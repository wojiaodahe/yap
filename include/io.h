#ifndef __IO_H__
#define __IO_H__

#define writeb(data, addr) ((*(volatile unsigned char *)(addr)) = (data))
#define readb(addr) ((*(volatile unsigned char *)(addr)))

#endif

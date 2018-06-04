#include <stdarg.h>
#include "common.h"

extern void memset(char *src, unsigned int num, unsigned int len);
extern void Uart_SendString(char *pt);
extern vsprintf(char *, const char *, va_list);

//=====================================================================
//If you don't use vsprintf(), the code size is reduced very much.
void printk(char *fmt, ...)
{
	
	va_list ap;
	char string[256];
	
	memset(string, 0, 256);
	
	va_start(ap, fmt);
	vsprintf(string, fmt, ap);
	
	__call_tty_drivers(string, strlen(string));	
	va_end(ap);
}

#include "printf.h"
#include "common.h"
#include "syscall.h"

const char *hexstr = "0123456789abcdef";
int hex2string(unsigned int hex, char *buf)
{
    unsigned int tmp = 0;
    unsigned int i = 0;
    char arr[10] = { 0 };

    do
    {
        tmp = hex & 0xf;
        arr[i++] = *(hexstr + tmp);
        hex = hex >> 4;
    }while (hex);

    tmp = i;
    while (i--)
        *buf++ = arr[i];

    return tmp;
}

int int2string(int num, char *buf)
{
    int i = 0;
    int tmp = 0;
    char sign = 0;
    char arr[16] = { 0 };

    if (num < 0)
    {
        sign = 1;
        num = (int)0 - num;
    }

    do 
    {
        tmp = num % 10;
        arr[i++] = *(hexstr + tmp);
        num /= 10;
    }while (num);

    if (sign)
        arr[i++] = '-';

    tmp = i;
    while (i--)
        *buf++ = arr[i];

    return tmp;
}

int flt2string(double flt, char *buf)
{
    return 0;
}

int vsprintk(char *buf, char *fmt, va_list vp)
{
    double vargflt = 0;
    int  vargint = 0;
    char* vargpch = NULL;
    char vargch = 0;
    char* pfmt = NULL;
    int offset = 0;

    pfmt = fmt;
    while(*pfmt)
    {
        if(*pfmt == '%')
        {
            switch(*(++pfmt))
            {

            case 'c':
                vargch = va_arg(vp, int);
                *(buf + offset) = vargch;
                offset++;
                break;
            case 'd':
            case 'i':
                vargint = va_arg(vp, int);
                offset += int2string(vargint, buf + offset);
                break;
            case 'f':
                vargflt = va_arg(vp, double);
                offset += flt2string(vargflt, buf + offset);
                break;
            case 's':
                vargpch = va_arg(vp, char*);
                while (*vargpch)
                {
                    *(buf + offset) = *vargpch++;
                    offset++;
                }
                break;
            case 'x':
            case 'X':
                vargint = va_arg(vp, int);
                offset += hex2string(vargint, buf + offset);
                break;
            case '%':
                *(buf + offset) = '%';
                offset++;
                break;
            default:
                break;
            }
            pfmt++;
        }
        else
        {
            *(buf + offset) = *pfmt++;
            offset++;
        }
    }

    return offset;
}

int sprintf(char *buf, char *fmt, ...)
{
	int len;
	va_list ap;

	va_start(ap, fmt);
	len = vsprintk(buf, fmt, ap);
	va_end(ap);

	return len;
}

int printf(char *fmt, ...)
{
	const int argc = 3;
	unsigned int argv[argc];
	int ret;

	va_list ap;
	char string[256];

    memset(string, 0, 256);
	
	va_start(ap, fmt);

	ret = vsprintk(string, fmt, ap);
	argv[0] = SYSTEM_CALL_PRINTF;
	argv[1] = (unsigned long)string;
	argv[2] = ret;
	ret = user_syscall(argc, argv);

	va_end(ap);

	return ret;
}

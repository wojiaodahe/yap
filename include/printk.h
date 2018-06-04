#ifndef __PRINTk_H__
#define __PRINTK_H__

void    print(char* fmt, ...);
void    printch(char ch);
void    printdec(int dec);
void    printflt(double flt);
void    printbin(int bin);
void    printhex(int hex);
void    printstr(char* str);

#define console_print(ch)    put_char(ch)
#endif

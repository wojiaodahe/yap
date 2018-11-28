#ifndef __SYSLIB_H__
#define __SYSLIB_H__


extern void memset(void *src, unsigned int num, unsigned int len);
extern void *memcpy(void *dest, void *src, unsigned int len);
extern int strncmp(char *s1, char *s2, unsigned int n);
extern int strcmp(char *s1, char *s2);
extern int strlen(char *);

#endif

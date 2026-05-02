#ifndef LIBC_STRING_H
#define LIBC_STRING_H

int strlen(const char *str);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
void *memset(void *s, int c, int n);
void *memcpy(void *dest, const void *src, int n);

#endif
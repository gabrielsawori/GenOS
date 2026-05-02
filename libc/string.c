#include "string.h"

// Menghitung panjang string
int strlen(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// Menyalin string dari src ke dest
char *strcpy(char *dest, const char *src) {
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

// Membandingkan dua string (mengembalikan 0 jika sama)
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Mengisi blok memori dengan nilai tertentu (sangat dibutuhkan untuk membersihkan RAM)
void *memset(void *s, int c, int n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

// Menyalin blok memori
void *memcpy(void *dest, const void *src, int n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}
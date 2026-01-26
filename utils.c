#include "utils.h"

void memory_copy(char *source, char *dest, int n_bytes) {
    for (int i = 0; i < n_bytes; i++) {
        *(dest + i) = *(source + i);
    }
}

int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i]; s[i] = s[j]; s[j] = c;
    }
}

void int_to_ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0) str[i++] = '-';
    str[i] = '\0';
    reverse(str);
}

// --- UPDATE v1.0: String Operations ---
int strcmp(char s1[], char s2[]) {
    int i; 
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

void strcpy(char src[], char dest[]) {
    int i = 0;
    while (src[i] != 0) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

void strcat(char dest[], char src[]) {
    int dest_len = strlen(dest);
    int i = 0;
    while (src[i] != 0) {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = 0;
}
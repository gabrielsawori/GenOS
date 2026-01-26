#ifndef UTILS_H
#define UTILS_H

#include "screen.h"

void memory_copy(char *source, char *dest, int n_bytes);
void int_to_ascii(int n, char str[]); 
int strlen(char s[]);
// --- UPDATE v1.0 ---
int strcmp(char s1[], char s2[]);
void strcpy(char src[], char dest[]);
void strcat(char dest[], char src[]);
// -------------------

#endif
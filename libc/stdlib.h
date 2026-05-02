#ifndef LIBC_STDLIB_H
#define LIBC_STDLIB_H

void *malloc(int size);
void free(void *ptr);
void *calloc(int num, int size); // Fungsi baru!

#endif
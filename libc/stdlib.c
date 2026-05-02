#include "stdlib.h"
#include "syscall.h"
#include "string.h" // Kita mengimpor pustaka string yang baru dibuat untuk menggunakan memset

#define SYS_MALLOC 90
#define SYS_FREE 91

void *malloc(int size) {
    return (void *)syscall(SYS_MALLOC, size, 0, 0);
}

void free(void *ptr) {
    syscall(SYS_FREE, (int)ptr, 0, 0);
}

// Meminjam memori dan langsung membersihkannya (menghindar dari bug data sisa)
void *calloc(int num, int size) {
    int total_size = num * size;
    void *ptr = malloc(total_size);
    if (ptr != 0) {
        memset(ptr, 0, total_size); // Bersihkan memori menjadi 0
    }
    return ptr;
}
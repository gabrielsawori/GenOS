#include "syscall.h"

// Menyembunyikan kerumitan Assembly dari aplikasi tingkat tinggi
int syscall(int eax, int ebx, int ecx, int edx) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(eax), "b"(ebx), "c"(ecx), "d"(edx)
    );
    return ret;
}
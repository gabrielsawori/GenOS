#include "stdio.h"
#include "syscall.h"
#include "string.h" // Kita panggil pustaka string yang baru di sini

// Syscall Nomor 4 adalah sys_write standar POSIX/Linux kita
#define SYS_WRITE 4

// (Fungsi strlen "sementara" sudah dihapus dari sini)

// Wrapper yang sangat bersih dan rapi!
void print(char *message) {
    // Sekarang ia akan menggunakan strlen resmi dari string.c
    syscall(SYS_WRITE, 1, (int)message, strlen(message));
}
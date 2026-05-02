#include "unistd.h"
#include "syscall.h"

// Nomor Syscall yang sudah kita daftarkan di kernel/syscall.c
#define SYS_OPEN 5
#define SYS_READ 3
#define SYS_CLOSE 6

int open(char *filename) {
    // Meminta Kernel membuka file dan mengembalikan File Descriptor (FD)
    return syscall(SYS_OPEN, (int)filename, 0, 0);
}

int read(int fd, char *buffer, int length) {
    // Meminta Kernel membaca isi file dari hard disk ke RAM (buffer)
    return syscall(SYS_READ, fd, (int)buffer, length);
}

int close(int fd) {
    // Meminta Kernel menutup file
    return syscall(SYS_CLOSE, fd, 0, 0);
}
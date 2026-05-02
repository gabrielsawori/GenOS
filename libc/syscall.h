#ifndef LIBC_SYSCALL_H
#define LIBC_SYSCALL_H

// Fungsi universal pembungkus Syscall untuk aplikasi User Space
int syscall(int eax, int ebx, int ecx, int edx);

#endif
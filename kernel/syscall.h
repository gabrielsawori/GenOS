#ifndef SYSCALL_H
#define SYSCALL_H

#include "utils.h"

// Daftar Nomor Syscall Standar (Meniru Linux x86)
#define SYS_EXIT    1
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6

// Struktur untuk menangkap status register saat interupsi terjadi
typedef struct {
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
} registers_t;

void syscall_init();
void syscall_handler(registers_t *regs);

#endif
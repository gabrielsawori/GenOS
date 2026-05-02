#ifndef GDT_H
#define GDT_H

#include "utils.h" // Untuk tipe data u8, u16, u32

void gdt_init();
void set_kernel_stack(u32 stack);

#endif
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "utils.h" // Untuk tipe data u8, u16, u32

void isr1_handler();
char get_ascii(u8 scancode);

// Fungsi baru untuk dipanggil oleh Syscall
int keyboard_read(char *buffer, int max_len);

#endif
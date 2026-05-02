// screen.h - Definisi fungsi layar
#ifndef SCREEN_H
#define SCREEN_H

// Definisi tipe data baku
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// Warna
#define COLOR_WHITE 0x07
#define COLOR_RED   0x04
#define COLOR_GREEN 0x02
#define COLOR_BLUE  0x01
#define COLOR_CYAN  0x03

// Daftar fungsi yang bisa dipanggil dari luar
void clear_screen();
void kprint(char *message);
void kprint_color(char *message, u8 color);
void kpanic(char *reason);

#endif
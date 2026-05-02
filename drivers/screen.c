// screen.c - Upgrade v0.8 (Scrolling)
#include "screen.h"
#include "utils.h" // Kita butuh memory_copy
#include "ports.h"

#define VGA_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

int cursor_pos = 0;

// Fungsi Internal: Cek apakah perlu scroll
void handle_scrolling() {
    // Jika kursor melewati batas layar
    if (cursor_pos >= MAX_ROWS * MAX_COLS * 2) {
        
        // Pindahkan Baris 1-24 ke atas (menimpa baris 0-23)
        int i;
        for (i = 1; i < MAX_ROWS; i++) {
            memory_copy(
                (char*)(VGA_ADDRESS + (i * MAX_COLS * 2)), 
                (char*)(VGA_ADDRESS + ((i - 1) * MAX_COLS * 2)), 
                MAX_COLS * 2
            );
        }

        // Kosongkan baris paling bawah (baris 24)
        char *last_line = (char*)(VGA_ADDRESS + ((MAX_ROWS - 1) * MAX_COLS * 2));
        for (i = 0; i < MAX_COLS * 2; i++) {
            last_line[i] = 0;
        }

        // Kembalikan kursor ke awal baris terakhir
        cursor_pos -= MAX_COLS * 2;
    }
}

void clear_screen() {
    volatile char *video = (volatile char*)VGA_ADDRESS;
    for (int i = 0; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        video[i] = ' ';
        video[i+1] = COLOR_WHITE;
    }
    cursor_pos = 0;
}

void kprint_color(char *message, u8 color) {
    volatile char *video = (volatile char*)VGA_ADDRESS;
    while (*message != 0) {
        if (*message == '\n') {
            int row = cursor_pos / (MAX_COLS * 2);
            cursor_pos = (row + 1) * (MAX_COLS * 2);
        } else if (*message == '\b') {
            if (cursor_pos > 0) {
                cursor_pos -= 2;
                video[cursor_pos] = ' ';
                video[cursor_pos+1] = color;
            }
        } else {
            video[cursor_pos] = *message;
            video[cursor_pos+1] = color;
            cursor_pos += 2;
        }
        
        handle_scrolling(); // <--- INI KUNCINYA
        message++;
    }
}

void kprint(char *message) { kprint_color(message, COLOR_WHITE); }

void kpanic(char *reason) {
    clear_screen();
    kprint_color("PANIC: ", 0x4F);
    kprint_color(reason, 0x4F);
    __asm__("hlt");
}
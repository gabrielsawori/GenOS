#include "keyboard.h"
#include "ports.h"
#include "screen.h"

#define KBD_BUFFER_SIZE 256
char kbd_buffer[KBD_BUFFER_SIZE];
int kbd_buffer_idx = 0;
volatile int kbd_input_ready = 0;
volatile int kbd_app_mode = 0; 

int shift_pressed = 0;
int caps_lock = 0;

extern void kernel_handle_input(char c);

char get_ascii(u8 scancode) {
    char map[] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, 
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    char shift_map[] = {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, 
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
    };

    if (scancode > 58) return 0;

    char c = map[scancode];
    char c_shift = shift_map[scancode];
    int is_letter = (c >= 'a' && c <= 'z');

    if (shift_pressed && !caps_lock) return c_shift;
    else if (!shift_pressed && caps_lock) return (is_letter ? c_shift : c);
    else if (shift_pressed && caps_lock) return (is_letter ? c : c_shift);

    return c;
}

void isr1_handler() {
    u8 scancode = port_byte_in(0x60);
    port_byte_out(0x20, 0x20); // Beri tahu PIC

    if (scancode & 0x80) {
        u8 break_code = scancode - 0x80;
        if (break_code == 0x2A || break_code == 0x36) shift_pressed = 0;
        return;
    }

    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return; }
    if (scancode == 0x3A) { caps_lock = !caps_lock; return; }

    char c = get_ascii(scancode);
    if (c != 0) {
        if (kbd_app_mode == 1) {
            // --- MODE APLIKASI AKTIF ---
            if (c == '\b') {
                if (kbd_buffer_idx > 0) {
                    kbd_buffer_idx--;
                    char str[2] = {c, '\0'}; kprint(str); 
                }
            } else {
                char str[2] = {c, '\0'};
                kprint(str); 
                if (kbd_buffer_idx < KBD_BUFFER_SIZE - 1) {
                    kbd_buffer[kbd_buffer_idx++] = c;
                }
            }
            if (c == '\n') kbd_input_ready = 1;
        } else {
            // --- MODE SHELL AKTIF ---
            kernel_handle_input(c);
        }
    }
}

int keyboard_read(char *buffer, int max_len) {
    kbd_app_mode = 1; 
    kbd_input_ready = 0;
    kbd_buffer_idx = 0;

    // Loop penahan dengan filter Anti Ghost-Enter
    while (1) {
        while (kbd_input_ready == 0) {
            // Memastikan interupsi tidak terblokir dan menghemat daya CPU
            __asm__ volatile("sti");
            __asm__ volatile("hlt");
        }
        
        // JIKA buffer HANYA berisi 'Enter' (sisa eksekusi sebelumnya), abaikan!
        if (kbd_buffer_idx == 1 && kbd_buffer[0] == '\n') {
            kbd_buffer_idx = 0;       // Reset buffer
            kbd_input_ready = 0;      // Turunkan bendera
            continue;                 // Lanjut menunggu ketikan asli
        }
        
        break; // Jika ada teksnya, keluar dari loop
    }

    // Salin isi buffer
    int copy_len = kbd_buffer_idx;
    if (copy_len > max_len) copy_len = max_len;
    for (int i = 0; i < copy_len; i++) {
        buffer[i] = kbd_buffer[i];
    }

    kbd_app_mode = 0; 
    return copy_len;
}
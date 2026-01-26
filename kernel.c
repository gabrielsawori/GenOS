#include "screen.h"
#include "utils.h"
#include "ports.h"
#include "hardware.h"
#include "fs.h"
#include "idt.h" // Wajib include ini!

#define ROOT_PASSWORD "admin"

// Global Variables
char key_buffer[256];
int buffer_idx = 0;
int is_logged_in = 0;

// Variabel Parsing
char cmd_main[32];
char cmd_arg[64];
char cmd_arg2[64];

// Forward Declaration
void exec_cmd();
char get_ascii(u8 scancode);

// --- INTERRUPT HANDLER (OTAK BARU KEYBOARD) ---
// Fungsi ini dipanggil otomatis oleh CPU saat ada tombol ditekan
void isr1_handler() {
    // 1. Baca data dari keyboard
    u8 scancode = port_byte_in(0x60);
    
    // 2. Kirim sinyal EOI (End of Interrupt) ke PIC
    // Ini wajib agar keyboard mau mengirim tombol berikutnya
    port_byte_out(0x20, 0x20);

    // 3. Proses Tombol (Logic yang dulu ada di while loop)
    if (scancode < 0x80) { // Key Press
        char c = get_ascii(scancode);
        
        if (c == '\n') { // ENTER
            if (!is_logged_in) {
                kprint("\n");
                if (strcmp(key_buffer, ROOT_PASSWORD) == 0) {
                    is_logged_in = 1;
                    kprint_color("Access Granted.\n\n", 0x02);
                } else {
                    kprint_color("Access Denied.\n", 0x04);
                }
                
                // Reset Buffer
                for(int i=0; i<256; i++) key_buffer[i]=0;
                buffer_idx=0;
                
                if(is_logged_in) kprint_color("root@server: ", 0x02);
                else kprint("Password: ");
            } else {
                exec_cmd(); // Eksekusi perintah
            }
        } 
        else if (c == '\b') { // BACKSPACE
           if (buffer_idx > 0) { 
               buffer_idx--; 
               key_buffer[buffer_idx]=0; 
               kprint("\b"); 
           }
        }
        else if (c != 0) { // KARAKTER BIASA
            if (buffer_idx < 255) {
                key_buffer[buffer_idx++] = c;
                
                // Masking Password
                if (!is_logged_in) kprint("*"); 
                else { 
                    char str[2] = {c, 0}; 
                    kprint(str); 
                }
            }
        }
    }
}

// --- LOGIKA SHELL (Sama seperti sebelumnya) ---
void parse_command() {
    int i = 0, j = 0;
    while(key_buffer[i] != ' ' && key_buffer[i] != 0) cmd_main[j++] = key_buffer[i++];
    cmd_main[j] = 0;
    
    if (key_buffer[i] == ' ') {
        i++; j = 0;
        while(key_buffer[i] != 0 && key_buffer[i] != ' ') cmd_arg[j++] = key_buffer[i++];
        cmd_arg[j] = 0;
    } else cmd_arg[0] = 0;
    
    if (key_buffer[i] == ' ') {
        i++; j = 0;
        while(key_buffer[i] != 0) cmd_arg2[j++] = key_buffer[i++];
        cmd_arg2[j] = 0;
    } else cmd_arg2[0] = 0;
}

void exec_cmd() {
    kprint("\n");
    parse_command();

    if (strcmp(cmd_main, "cls") == 0) clear_screen();
    else if (strcmp(cmd_main, "help") == 0) {
        kprint("System: cls, date, beep, panic, logout\n");
        kprint("FS: ls, touch <name>, rm <name>, cat <name>, write <name> <text>\n");
    }
    // ... Command System ...
    else if (strcmp(cmd_main, "date") == 0) {
        char time_str[10]; get_time_string(time_str);
        kprint("Time: "); kprint_color(time_str, 0x03); kprint("\n");
    }
    else if (strcmp(cmd_main, "beep") == 0) { kprint("Beep!\n"); beep(); }
    else if (strcmp(cmd_main, "panic") == 0) kpanic("Test Crash");
    else if (strcmp(cmd_main, "logout") == 0) {
        is_logged_in = 0; clear_screen(); kprint("Logged Out.\nPassword: ");
    }
    // ... Command FS ...
    else if (strcmp(cmd_main, "ls") == 0) fs_list();
    else if (strcmp(cmd_main, "touch") == 0) fs_create(cmd_arg);
    else if (strcmp(cmd_main, "rm") == 0) fs_delete(cmd_arg);
    else if (strcmp(cmd_main, "cat") == 0) fs_read(cmd_arg);
    else if (strcmp(cmd_main, "write") == 0) fs_write(cmd_arg, cmd_arg2);
    
    else if (cmd_main[0] != 0) {
        kprint("Unknown command: "); kprint(cmd_main); kprint("\n");
    }

    // Reset Buffer
    for(int i=0; i<256; i++) key_buffer[i] = 0;
    buffer_idx = 0;
    
    if (is_logged_in) kprint_color("root@server: ", 0x02);
    else if (strcmp(cmd_main, "logout") != 0) kprint("Password: "); 
}

char get_ascii(u8 scancode) {
    char map[] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, 
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    if (scancode > 58) return 0;
    return map[scancode];
}

// --- MAIN KERNEL ---
void kernel_main() {
    clear_screen();
    fs_init();
    
    // 1. Inisialisasi Interrupt Descriptor Table
    // Ini memasang "kabel" keyboard ke isr1_handler
    set_idt();
    
    // 2. Aktifkan Interrupt di CPU (sti = Set Interrupt Flag)
    // Tanpa ini, keyboard tidak akan merespon
    __asm__("sti");

    kprint_color(" # SERVER OS v1.1 #\n", 0x03);
    kprint("Interrupts: "); kprint_color("Enabled (IRQ Driven)\n", 0x02);
    kprint("System Ready.\n\n");
    kprint("Password: ");

    // 3. Loop Utama (Sekarang Kosong!)
    // CPU tidak perlu lagi capek-capek tanya keyboard.
    // Kita suruh CPU tidur (hlt) untuk hemat daya.
    // Saat tombol ditekan, CPU bangun, jalankan isr1_handler, lalu tidur lagi.
    while (1) {
        __asm__("hlt"); 
    }
}
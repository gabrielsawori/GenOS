#include "screen.h"
#include "utils.h"
#include "ports.h"
#include "hardware.h"
#include "fs.h"
#include "idt.h" 
#include "user.h"
#include "ata.h"
#include "mem.h"
#include "gdt.h" 
#include "syscall.h" 
#include "elf.h" 
#include "../apps/app_data.h" 
#include "pci.h"
#include "e1000.h"

// Global Variables untuk Shell
char key_buffer[256];
int buffer_idx = 0;
int login_state = 0; 
char temp_username[32];
char cmd_main[32];
char cmd_arg[64];
char cmd_arg2[64];

// Forward Declaration
void exec_cmd();

int simple_atoi(char *str) {
    int res = 0;
    int i = 0;
    while(str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }
    return res;
}

// --- PENERIMA INPUT DARI DRIVER KEYBOARD ---
void kernel_handle_input(char c) {
    if (c == '\n') { 
        kprint("\n");
        if (login_state == 0) { 
            strcpy(temp_username, key_buffer);
            for(int i=0; i<256; i++) key_buffer[i]=0;
            buffer_idx=0;
            login_state = 1;
            kprint("Password: ");
        } 
        else if (login_state == 1) { 
            if (user_login(temp_username, key_buffer)) {
                login_state = 2;
                kprint_color("Access Granted.\n\n", 0x02);
            } else {
                kprint_color("Access Denied.\n", 0x04);
                login_state = 0;
            }
            for(int i=0; i<256; i++) key_buffer[i]=0;
            buffer_idx=0;
            
            if (login_state == 2) {
                kprint_color(user_get_current_user(), 0x02);
                kprint_color("@server", 0x02);
                kprint(":");
                char cwd[64];
                fs_get_cwd(cwd);
                kprint_color(cwd, 0x03);
                kprint("$ ");
            } else {
                kprint("Login: ");
            }
        } 
        else { 
            exec_cmd(); 
        }
    } 
    else if (c == '\b') { 
        if (buffer_idx > 0) { 
            buffer_idx--; 
            key_buffer[buffer_idx]=0; 
            kprint("\b"); 
        }
    }
    else { 
        if (buffer_idx < 255) {
            key_buffer[buffer_idx++] = c;
            if (login_state == 1) kprint("*"); 
            else { char str[2] = {c, 0}; kprint(str); }
        }
    }
}

// --- LOGIKA PARSING SHELL ---
void parse_command() {
    int i = 0, j = 0;
    // Reset buffer perintah
    for(int k=0; k<32; k++) cmd_main[k] = 0;
    for(int k=0; k<64; k++) cmd_arg[k] = 0;
    for(int k=0; k<64; k++) cmd_arg2[k] = 0;

    // Ambil perintah utama
    while(key_buffer[i] != ' ' && key_buffer[i] != 0) cmd_main[j++] = key_buffer[i++];
    cmd_main[j] = 0;

    // Ambil argumen pertama
    if (key_buffer[i] == ' ') {
        i++; j = 0;
        while(key_buffer[i] != 0 && key_buffer[i] != ' ') cmd_arg[j++] = key_buffer[i++];
        cmd_arg[j] = 0;
    }

    // Ambil argumen kedua
    if (key_buffer[i] == ' ') {
        i++; j = 0;
        while(key_buffer[i] != 0) cmd_arg2[j++] = key_buffer[i++];
        cmd_arg2[j] = 0;
    }
}

// --- EKSEKUSI PERINTAH SHELL ---
void exec_cmd() {
    parse_command();

    if (strcmp(cmd_main, "cls") == 0) {
        clear_screen();
    }
    else if (strcmp(cmd_main, "help") == 0) {
        kprint("System: cls, date, beep, logout, whoami, mem_test, sys_test, net_test\n");
        kprint("FS: ls, cd, pwd, mkdir, touch, rm, cat, write, exec <file>\n");
        kprint("User: useradd <user> <pass>\n");
    }
    else if (strcmp(cmd_main, "logout") == 0) {
        login_state = 0; 
        clear_screen(); 
        kprint("Logged Out.\nLogin: ");
    }
    else if (strcmp(cmd_main, "mem_test") == 0) {
        kprint("Menguji PMM dan Heap Allocator...\n");
        void* p = kmalloc(1024);
        if (p) { 
            kprint_color("[SUKSES] Alokasi memori 1KB berhasil!\n", 0x02); 
            kfree(p); 
        } else {
            kprint_color("[GAGAL] Alokasi memori gagal!\n", 0x04);
        }
    }
    else if (strcmp(cmd_main, "net_test") == 0) {
        kprint("Merakit dan menembakkan paket uji coba (Broadcast)...\n");
        
        u8 frame[64]; 
        
        // 1. Alamat Tujuan: Broadcast (FF:FF:FF:FF:FF:FF)
        for(int i=0; i<6; i++) frame[i] = 0xFF;
        
        // 2. Alamat Pengirim: MAC Address GenOS kita
        for(int i=0; i<6; i++) frame[i+6] = mac_address[i];
        
        // 3. Tipe Protokol: 0x1337 (Sandi khusus GenOS Test)
        frame[12] = 0x13; 
        frame[13] = 0x37;
        
        // 4. Isi Pesan (Payload)
        char *msg = "HELLO DARI GENOS TX!";
        int p = 0;
        while(msg[p] != 0) { frame[14+p] = msg[p]; p++; }
        
        // Padding agar ukuran paket minimal 64 byte
        for(int i = 14 + p; i < 64; i++) frame[i] = 0;

        // Kirim ke kartu jaringan!
        e1000_send_packet(frame, 64);
        kprint_color("[SUKSES] Paket telah dikirim ke kawat jaringan!\n", 0x02);
    }
    else if (strcmp(cmd_main, "exec") == 0) {
        if (cmd_arg[0] != 0) execute_elf(cmd_arg);
        else kprint_color("Usage: exec <filename>\n", 0x04);
    }
    else if (strcmp(cmd_main, "ls") == 0) fs_list();
    else if (strcmp(cmd_main, "pwd") == 0) fs_pwd();
    else if (cmd_main[0] != 0) {
        kprint("Unknown command: "); kprint(cmd_main); kprint("\n");
    }

    // Reset buffer input shell
    for(int i=0; i<256; i++) key_buffer[i] = 0;
    buffer_idx = 0;
    
    // Cetak kembali prompt jika masih dalam keadaan login
    if (login_state == 2) {
        kprint_color(user_get_current_user(), 0x02);
        kprint_color("@server", 0x02);
        kprint(":");
        char cwd[64]; 
        fs_get_cwd(cwd);
        kprint_color(cwd, 0x03); 
        kprint("$ ");
    }
}

// --- FUNGSI UTAMA KERNEL (ENTRY POINT) ---
void kernel_main() {
    clear_screen();
    
    // 1. Inisialisasi Fondasi Perangkat Lunak & CPU
    gdt_init(); 
    set_idt();      // Aktifkan IDT Gate
    __asm__("sti"); // Aktifkan Interupsi CPU
    
    mem_init();     // Physical Memory Manager (PMM) & Heap Aktif
    fs_init();      // File System Mount
    user_init();    // User Management Aktif
    syscall_init(); // Antarmuka Syscall Aktif

    kprint_color(" # GENOS v2.5 (Kernel Hardened & Network Ready) #\n", 0x03);
    
    // 2. Inisialisasi Perangkat Keras Jaringan
    pci_scan();   // Radar PCI memindai bus motherboard
    e1000_init(); // Nyalakan driver Intel E1000 (MAC, Ring Buffers, & IRQ)

    kprint("\nLogin: ");

    // Loop abadi untuk menjaga CPU tetap hidup (menunggu interupsi)
    while (1) {
        __asm__("hlt"); 
    }
}
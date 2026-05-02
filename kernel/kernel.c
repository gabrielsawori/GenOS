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
#include "net.h"

// Global Variables
char key_buffer[256];
int buffer_idx = 0;
int login_state = 2; // <--- BYPASS AKTIF: Kamu akan langsung masuk ke Shell
char temp_username[32];
char cmd_main[32];
char cmd_arg[64];
char cmd_arg2[64];

void exec_cmd();

// --- HANDLER KEYBOARD ---
void kernel_handle_input(char c) {
    if (c == '\n') { 
        kprint("\n");
        if (login_state == 0) { 
            // PERBAIKAN: strcpy(temp_username, key_buffer)
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
                kprint("Login: ");
            }
            for(int i=0; i<256; i++) key_buffer[i]=0;
            buffer_idx=0;
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

// --- LOGIKA SHELL ---
void parse_command() {
    int i = 0, j = 0;
    for(int k=0; k<32; k++) cmd_main[k] = 0;
    for(int k=0; k<64; k++) cmd_arg[k] = 0;
    for(int k=0; k<64; k++) cmd_arg2[k] = 0;

    while(key_buffer[i] != ' ' && key_buffer[i] != 0) cmd_main[j++] = key_buffer[i++];
    cmd_main[j] = 0;
    if (key_buffer[i] == ' ') {
        i++; j = 0;
        while(key_buffer[i] != 0 && key_buffer[i] != ' ') cmd_arg[j++] = key_buffer[i++];
        cmd_arg[j] = 0;
    }
    if (key_buffer[i] == ' ') {
        i++; j = 0;
        while(key_buffer[i] != 0) cmd_arg2[j++] = key_buffer[i++];
        cmd_arg2[j] = 0;
    }
}

void exec_cmd() {
    parse_command();

    if (strcmp(cmd_main, "cls") == 0) clear_screen();
    else if (strcmp(cmd_main, "help") == 0) {
        kprint("System: cls, help, logout, ping, net_test, mem_test\n");
        kprint("FS: ls, cd, pwd, cat, write, exec <file>\n");
    }
    else if (strcmp(cmd_main, "ping") == 0) net_send_ping();
    else if (strcmp(cmd_main, "net_test") == 0) {
        u8 frame[64];
        for(int i=0; i<6; i++) frame[i] = 0xFF;
        for(int i=0; i<6; i++) frame[i+6] = mac_address[i];
        frame[12] = 0x13; frame[13] = 0x37;
        e1000_send_packet(frame, 64);
        kprint("[SUKSES] Paket uji coba dikirim.\n");
    }
    else if (strcmp(cmd_main, "ls") == 0) fs_list();
    else if (strcmp(cmd_main, "pwd") == 0) fs_pwd();
    else if (strcmp(cmd_main, "logout") == 0) { login_state = 0; clear_screen(); kprint("Login: "); }
    else if (cmd_main[0] != 0) { kprint("Unknown command\n"); }

    for(int i=0; i<256; i++) key_buffer[i] = 0;
    buffer_idx = 0;
    
    if (login_state == 2) {
        kprint_color("root@server", 0x02);
        kprint(":");
        char cwd[64]; fs_get_cwd(cwd);
        kprint_color(cwd, 0x03); kprint("$ ");
    }
}

void kernel_main() {
    clear_screen();
    gdt_init(); 
    set_idt();
    __asm__("sti"); 
    mem_init();
    fs_init();
    user_init(); 
    syscall_init(); 

    kprint_color(" # GENOS v2.5 (Network Ready) #\n", 0x03);
    pci_scan();   
    e1000_init(); 
    net_init();

    if (login_state == 2) {
        kprint_color("root@server", 0x02); kprint(": / $ ");
    } else {
        kprint("\nLogin: ");
    }

    while (1) { __asm__("hlt"); }
}
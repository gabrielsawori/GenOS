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
#include "pci.h"
#include "e1000.h"
#include "net.h"
#include "paging.h"
#include "timer.h"

// Global Variables for Shell
char key_buffer[256];
int buffer_idx = 0;
int login_state = 2; // BYPASS: Directly enter root for easier debugging
char temp_username[32];
char cmd_main[32];
char cmd_arg[64];
char cmd_arg2[64];

// Forward Declaration
void exec_cmd();

// --- FUNGSI BARU: Mengembalikan Prompt Shell (Dibutuhkan oleh syscall.c) ---
void shell_loop() {
    for(int i=0; i<256; i++) key_buffer[i] = 0;
    buffer_idx = 0;
    
    if (login_state == 2) {
        kprint_color(user_get_current_user(), 0x02);
        kprint_color("@server", 0x02);
        kprint(":");
        char cwd[64]; fs_get_cwd(cwd);
        kprint_color(cwd, 0x03); 
        kprint("$ ");
    } else {
        kprint("Login: ");
    }
}
// -------------------------------------------------------------------------

// --- KEYBOARD INPUT HANDLER ---
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
            shell_loop(); // Panggil fungsi shell_loop
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

// --- SHELL PARSING LOGIC ---
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

// --- SHELL COMMAND EXECUTION ---
void exec_cmd() {
    parse_command();

    if (strcmp(cmd_main, "cls") == 0) clear_screen();
    else if (strcmp(cmd_main, "help") == 0) {
        kprint("System: cls, help, logout, ping, net_test, mem_test\n");
        kprint("FS & App: ls, pwd, exec <file>\n");
        kprint("User: useradd <user> <pass>\n");
    }
    else if (strcmp(cmd_main, "ping") == 0) {
        net_send_ping(); 
    }
    else if (strcmp(cmd_main, "net_test") == 0) {
        kprint("Sending broadcast test packet...\n");
        u8 frame[64]; 
        for(int i=0; i<6; i++) frame[i] = 0xFF;
        for(int i=0; i<6; i++) frame[i+6] = mac_address[i];
        frame[12] = 0x13; frame[13] = 0x37;
        e1000_send_packet(frame, 64);
        kprint("[SUCCESS] Packet sent.\n");
    }
    else if (strcmp(cmd_main, "mem_test") == 0) {
        kprint("Testing memory allocation...\n");
        void* p = kmalloc(1024);
        if (p) { kprint_color("[SUCCESS] 1KB OK!\n", 0x02); kfree(p); }
        else kprint_color("[FAILED]\n", 0x04);
    }
    else if (strcmp(cmd_main, "exec") == 0) { 
        if (cmd_arg[0] != 0) execute_elf(cmd_arg);
        else kprint_color("Usage: exec <filename>\n", 0x04);
    }
    else if (strcmp(cmd_main, "useradd") == 0) { 
        if (cmd_arg[0] != 0 && cmd_arg2[0] != 0) {
            user_add(cmd_arg, cmd_arg2);
            kprint_color("[SUCCESS] New user registered.\n", 0x02);
        } else {
            kprint_color("Usage: useradd <username> <password>\n", 0x04);
        }
    }
    else if (strcmp(cmd_main, "ls") == 0) fs_list();
    else if (strcmp(cmd_main, "pwd") == 0) fs_pwd();
    else if (strcmp(cmd_main, "logout") == 0) {
        login_state = 0; 
        clear_screen(); 
    }
    else if (cmd_main[0] != 0) {
        kprint("Unknown command: "); kprint(cmd_main); kprint("\n");
    }

    shell_loop(); // Selalu panggil ini setelah command selesai untuk mencetak prompt
}

// --- KERNEL MAIN FUNCTION (ENTRY POINT) ---
void kernel_main() {
    clear_screen();
    
    // 1. Basic CPU & Physical Memory Initialization
    gdt_init(); 
    set_idt();      
    __asm__("sti"); 
    mem_init();     
    
    // 2. HARDWARE DETECTION 
    pci_scan();   
    
    // 3. ENABLE PAGING 
    paging_init();  
    
    // 4. Software System & Abstraction Initialization
    fs_init();      
    user_init();    
    syscall_init(); 

    kprint_color(" # GENOS v2.9 (Grand Finale: Python Ready) #\n", 0x03);
    
    // 5. Network Driver Initialization
    e1000_init(); 
    net_init();   
    
    // 6. START THE HEARTBEAT (1000x per second)
    timer_init(1000); 

    // Initial Prompt dipanggil melalui shell_loop()
    shell_loop();

    // Main Loop (Idle)
    while (1) {
        __asm__("hlt"); 
    }
}
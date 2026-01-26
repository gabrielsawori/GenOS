#include "screen.h"
#include "utils.h"
#include "ports.h"
#include "hardware.h"
#include "fs.h"
#include "idt.h" 
#include "user.h"

// Global Variables
char key_buffer[256];
int buffer_idx = 0;

// Login State
// 0: Wait Username, 1: Wait Password, 2: Logged In
int login_state = 0; 
char temp_username[32];

// Variabel Parsing
char cmd_main[32];
char cmd_arg[64];
char cmd_arg2[64];

// Forward Declaration
void exec_cmd();
char get_ascii(u8 scancode);
void prompt_login();

// Simple atoi implementation
int simple_atoi(char *str) {
    int res = 0;
    int i = 0;
    while(str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }
    return res;
}

// --- INTERRUPT HANDLER (OTAK BARU KEYBOARD) ---
void isr1_handler() {
    u8 scancode = port_byte_in(0x60);
    port_byte_out(0x20, 0x20);

    if (scancode < 0x80) { // Key Press
        char c = get_ascii(scancode);
        
        if (c == '\n') { // ENTER
            kprint("\n");
            
            if (login_state == 0) { // Input Username
                strcpy(key_buffer, temp_username);
                // Reset Buffer
                for(int i=0; i<256; i++) key_buffer[i]=0;
                buffer_idx=0;
                
                login_state = 1;
                kprint("Password: ");
            } 
            else if (login_state == 1) { // Input Password
                if (user_login(temp_username, key_buffer)) {
                    login_state = 2;
                    kprint_color("Access Granted.\n\n", 0x02);
                } else {
                    kprint_color("Access Denied.\n", 0x04);
                    login_state = 0;
                }
                
                // Reset Buffer
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
            else { // Logged In
                exec_cmd(); 
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
                
                // Masking Password only in state 1
                if (login_state == 1) kprint("*"); 
                else { 
                    char str[2] = {c, 0}; 
                    kprint(str); 
                }
            }
        }
    }
}

// --- LOGIKA SHELL ---
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
    parse_command();

    if (strcmp(cmd_main, "cls") == 0) clear_screen();
    else if (strcmp(cmd_main, "help") == 0) {
        kprint("System: cls, date, beep, panic, logout, whoami, calc\n");
        kprint("FS: ls, cd, pwd, mkdir, touch, rm, cat, write\n");
        kprint("User: useradd <user> <pass>\n");
    }
    // ... Command System ...
    else if (strcmp(cmd_main, "date") == 0) {
        char time_str[10]; get_time_string(time_str);
        kprint("Time: "); kprint_color(time_str, 0x03); kprint("\n");
    }
    else if (strcmp(cmd_main, "beep") == 0) { kprint("Beep!\n"); beep(); }
    else if (strcmp(cmd_main, "panic") == 0) kpanic("Test Crash");
    else if (strcmp(cmd_main, "logout") == 0) {
        login_state = 0; clear_screen(); kprint("Logged Out.\nLogin: ");
    }
    else if (strcmp(cmd_main, "whoami") == 0) {
        kprint(user_get_current_user()); kprint("\n");
    }
    else if (strcmp(cmd_main, "calc") == 0) {
        // format: calc 5 + 3
        // cmd_arg is first number
        // cmd_arg2 is operator? No, parsing logic:
        // cmd_main = calc
        // cmd_arg = 5
        // cmd_arg2 = +
        // wait, simple parser only handles 3 tokens?
        // parse_command:
        // arg1 stops at space. arg2 takes rest?
        // Let's check parse_command again.
        // Arg2 takes rest. So if "calc 5 + 3", arg2="+"? No "calc 5 + 3"
        // key_buffer: "calc 5 + 3"
        // cmd_main: "calc"
        // cmd_arg: "5"
        // cmd_arg2: "+ 3"
        
        // My parser is weak. Let's rely on simple 2 args for now, e.g. "calc 5+" -> 5+ what?
        // Let's implement simpler: "add 5 3", "sub 5 3".
        // Or just handle "calc 5 3" -> 5+3.
        // Let's stick to "calc <num> <num>" and always add? No that's lame.
        
        // Let's parse cmd_arg2 manually for op and num2.
        int n1 = simple_atoi(cmd_arg);
        char op = cmd_arg2[0];
        int n2 = 0;
        
        // Find second number in cmd_arg2
        int k = 1;
        while(cmd_arg2[k] == ' ') k++;
        n2 = simple_atoi(&cmd_arg2[k]);
        
        int res = 0;
        if (op == '+') res = n1 + n2;
        else if (op == '-') res = n1 - n2;
        else if (op == '*') res = n1 * n2;
        else if (op == '/') {
             if (n2 != 0) res = n1 / n2;
        }
        
        char res_str[32];
        int_to_ascii(res, res_str);
        kprint("Result: "); kprint(res_str); kprint("\n");
    }
    else if (strcmp(cmd_main, "useradd") == 0) {
        if (strcmp(user_get_current_user(), "root") == 0) {
            user_add(cmd_arg, cmd_arg2);
        } else {
            kprint_color("Permission denied.\n", 0x04);
        }
    }
    // ... Command FS ...
    else if (strcmp(cmd_main, "ls") == 0) fs_list();
    else if (strcmp(cmd_main, "touch") == 0) fs_create(cmd_arg);
    else if (strcmp(cmd_main, "mkdir") == 0) fs_mkdir(cmd_arg);
    else if (strcmp(cmd_main, "cd") == 0) fs_cd(cmd_arg);
    else if (strcmp(cmd_main, "pwd") == 0) fs_pwd();
    else if (strcmp(cmd_main, "rm") == 0) fs_delete(cmd_arg);
    else if (strcmp(cmd_main, "cat") == 0) fs_read(cmd_arg);
    else if (strcmp(cmd_main, "write") == 0) fs_write(cmd_arg, cmd_arg2);
    
    else if (cmd_main[0] != 0) {
        kprint("Unknown command: "); kprint(cmd_main); kprint("\n");
    }

    // Reset Buffer
    for(int i=0; i<256; i++) key_buffer[i] = 0;
    buffer_idx = 0;
    
    if (login_state == 2) {
        kprint_color(user_get_current_user(), 0x02);
        kprint_color("@server", 0x02);
        kprint(":");
        char cwd[64];
        fs_get_cwd(cwd);
        kprint_color(cwd, 0x03);
        kprint("$ ");
    }
    else if (login_state == 0) kprint("Login: "); 
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
    user_init(); // Init User System
    
    set_idt();
    __asm__("sti");

    kprint_color(" # GENOS v2.0 (Enhanced) #\n", 0x03);
    kprint("Interrupts: "); kprint_color("Enabled\n", 0x02);
    kprint("Multi-user & Directory support enabled.\n\n");
    
    kprint("Login: ");

    while (1) {
        __asm__("hlt"); 
    }
}

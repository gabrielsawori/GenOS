#include "syscall.h"
#include "screen.h"
#include "mem.h" 
#include "fs.h" 
#include "keyboard.h" 
#include "idt.h" 

#define MAX_SYSCALLS 256

// Tipe data untuk fungsi syscall
typedef void (*syscall_handler_t)(registers_t *);

// Array penyimpan daftar fungsi syscall (Arsitektur Linux Asli!)
syscall_handler_t syscalls[MAX_SYSCALLS];

// Deklarasi eksternal untuk kembali ke Shell setelah program selesai
extern void shell_loop();

// --- 1. IMPLEMENTASI SYSCALL DASAR ---

void sys_write(registers_t *regs) {
    // EBX = File Descriptor, ECX = Pointer Teks
    int fd = regs->ebx;
    char *str = (char *)regs->ecx;
    
    if (fd == 1) { // 1 = stdout (Layar)
        kprint(str);
    }
}

void sys_exit(registers_t *regs) {
    // Menghentikan program
    kprint_color("\n[Program Terminated by Syscall Exit]\n", 0x0B);
    
    // PERBAIKAN: Mencegah OS Freeze dengan mengembalikan eksekusi ke Shell
    shell_loop(); 
}

// --- 2. IMPLEMENTASI SYSCALL MEMORI ---

void sys_malloc(registers_t *regs) {
    // EBX = Ukuran memori yang diminta
    u32 size = regs->ebx;
    // Mengembalikan alamat memori ke EAX
    regs->eax = (u32) kmalloc(size);
}

void sys_free(registers_t *regs) {
    // EBX = Alamat memori yang ingin dibebaskan
    void *ptr = (void *) regs->ebx;
    kfree(ptr);
}

// --- 3. IMPLEMENTASI SYSCALL FILE ---

void sys_open(registers_t *regs) {
    char *filename = (char *)regs->ebx;
    int file_id = fs_find_file(filename);
    
    if (file_id >= 0) {
        regs->eax = file_id + 10; // Offset 10 agar tidak bentrok dengan stdin/stdout
    } else {
        regs->eax = -1; // File tidak ditemukan
    }
}

void sys_read(registers_t *regs) {
    int fd = regs->ebx;
    u8 *buffer = (u8 *)regs->ecx;
    u32 length = regs->edx;

    if (fd == 0) {
        // File Descriptor 0 adalah STDIN (Keyboard)
        regs->eax = keyboard_read((char *)buffer, length);
    } else if (fd >= 10) { 
        // File Descriptor >= 10 adalah File dari Hard Disk
        int file_id = fd - 10;
        int bytes_read = fs_read_file_to_buffer(file_id, buffer, length);
        regs->eax = bytes_read; 
    } else {
        regs->eax = -1;
    }
}

void sys_close(registers_t *regs) {
    // Implementasi sederhana: kembalikan 0 (sukses)
    regs->eax = 0;
}

// --- 4. OTAK DISPATCHER ---

void syscall_handler(registers_t *regs) {
    // EAX menyimpan nomor Syscall
    u32 syscall_no = regs->eax;
    
    if (syscall_no < MAX_SYSCALLS && syscalls[syscall_no] != 0) {
        syscalls[syscall_no](regs); // Lompat ke fungsi yang tepat secara dinamis!
    } else {
        kprint_color("Error: Invalid System Call Number!\n", 0x04);
    }
}

// Assembly Wrapper untuk menangkap int 0x80 dengan aman
void syscall_dispatcher();
__asm__(
    ".global syscall_dispatcher\n"
    "syscall_dispatcher:\n"
    "pusha\n"               
    "push %esp\n"           
    "call syscall_handler\n"
    "add $4, %esp\n"        
    "popa\n"                
    "iret\n"                
);

void syscall_init() {
    // Bersihkan Array
    for(int i=0; i<MAX_SYSCALLS; i++) syscalls[i] = 0;
    
    // Mendaftarkan fungsi ke dalam Dispatch Table
    syscalls[SYS_EXIT]  = sys_exit;
    syscalls[SYS_READ]  = sys_read;
    syscalls[SYS_WRITE] = sys_write;
    syscalls[SYS_OPEN]  = sys_open;
    syscalls[SYS_CLOSE] = sys_close;
    syscalls[SYS_MALLOC] = sys_malloc;
    syscalls[SYS_FREE]  = sys_free;

    // Pasang ke CPU IDT
    set_idt_gate(0x80, (u32)syscall_dispatcher);
    kprint_color("[SUCCESS] Advanced Syscall Bridge Initialized.\n", 0x02);
}
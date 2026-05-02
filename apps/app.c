#include "../libc/stdio.h"
#include "../libc/unistd.h"
#include "../libc/syscall.h"

void _start() {
    char nama[50];
    
    print("\n=== PROGRAM INTERAKTIF GENOS ===\n");
    print("Siapa namamu, Engineer? ");
    
    // File Descriptor 0 akan membuat aplikasi berhenti dan menunggu ketikanmu!
    int bytes = read(0, nama, 50);
    
    if (bytes > 0) {
        nama[bytes-1] = '\0'; // Menghapus karakter enter (\n) yang ikut terbaca
        
        print("Salam kenal, ");
        print(nama);
        print("! Selamat datang di era OS interaktif!\n");
    }
    
    syscall(1, 0, 0, 0); // Exit
    while(1);
}
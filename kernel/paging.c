#include "paging.h"
#include "screen.h"

// Ambil koordinat MMIO dari pci.c
extern u32 e1000_mmio_base; 

u32 page_directory[1024] __attribute__((aligned(4096)));
u32 first_page_table[1024] __attribute__((aligned(4096)));
u32 mmio_page_table[1024] __attribute__((aligned(4096))); // PETA BARU UNTUK HARDWARE!

void load_page_directory(u32* dir) {
    __asm__ volatile("mov %0, %%cr3":: "r"(dir));
}

void enable_paging() {
    u32 cr0;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0":: "r"(cr0));
}

void paging_init() {
    kprint("=== INISIALISASI VIRTUAL MEMORY (PAGING) ===\n");

    for(int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002; 
    }

    // 1. Petakan 4MB pertama RAM (Tempat Kernel & Layar hidup)
    for(unsigned int i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 4096) | 3; 
    }
    page_directory[0] = ((u32)first_page_table) | 3;

    // 2. Petakan 4MB area MMIO Intel E1000 (Jika terdeteksi)
    if (e1000_mmio_base != 0) {
        // Cari tahu di bab (index) berapa MMIO berada (Geser 22 bit)
        u32 dir_idx = e1000_mmio_base >> 22; 
        u32 start_address = dir_idx * 0x400000; 
        
        for(unsigned int i = 0; i < 1024; i++) {
            mmio_page_table[i] = (start_address + (i * 4096)) | 3;
        }
        
        // Masukkan peta MMIO ke Buku Utama
        page_directory[dir_idx] = ((u32)mmio_page_table) | 3;
        kprint_color("[PAGING] Area MMIO Jaringan Berhasil Dipetakan!\n", 0x03);
    }

    kprint("Memuat Page Directory ke CR3...\n");
    load_page_directory(page_directory);
    
    kprint("Menyalakan MMU di CR0...\n");
    enable_paging();

    kprint_color("[SUKSES] Paging Aktif! RAM GenOS kini dikendalikan oleh MMU.\n\n", 0x02);
}
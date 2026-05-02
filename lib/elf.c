#include "elf.h"
#include "fs.h"
#include "screen.h"
#include "mem.h" // Butuh memori dinamis untuk memuat file dari disk
#include "ata.h" // Butuh driver disk

// Import struktur FAT dan Directory dari fs.c
extern dir_entry_t directory[];
extern u32 fat[];
extern int fs_find_file(char *name);

// Nilai standar dari fs.h
#define FAT_EOF 0xFFFFFFFF
#define FAT_FREE 0x00000000
#define SECTOR_SIZE 512

void execute_elf(char *filename) {
    // 1. Cari file di Sistem Berkas FAT
    int file_id = fs_find_file(filename);
    if (file_id == -1) {
        kprint_color("Error: Executable file not found.\n", 0x04);
        return;
    }

    u32 file_size = directory[file_id].size;
    u32 first_sec = directory[file_id].first_sector;

    if (file_size == 0 || first_sec == 0) {
        kprint_color("Error: File is empty.\n", 0x04);
        return;
    }

    // 2. Alokasikan Memori Dinamis (Heap) sebesar ukuran file
    u8 *file_content = (u8*) kmalloc(file_size);
    if (file_content == 0) {
        kprint_color("Error: Not enough memory to load ELF.\n", 0x04);
        return;
    }

    kprint("Loading file from Disk to RAM...\n");

    // 3. Baca rantaian file dari Hard Disk ke RAM yang sudah dialokasikan
    u32 curr_sec = first_sec;
    u32 bytes_read = 0;
    u8 sector_buffer[SECTOR_SIZE];

    while (curr_sec != FAT_EOF && curr_sec != FAT_FREE) {
        ata_read_sector(curr_sec, sector_buffer);

        u32 copy_len = SECTOR_SIZE;
        if (file_size - bytes_read < SECTOR_SIZE) {
            copy_len = file_size - bytes_read;
        }

        // Salin isi sektor ke buffer aplikasi
        for (u32 i = 0; i < copy_len; i++) {
            file_content[bytes_read + i] = sector_buffer[i];
        }

        bytes_read += copy_len;
        curr_sec = fat[curr_sec]; // Lompat ke mata rantai (sektor) berikutnya
    }

    // 4. Cek "Magic Number" ELF (\x7f E L F)
    elf32_ehdr_t *hdr = (elf32_ehdr_t *) file_content;
    
    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' ||
        hdr->e_ident[2] != 'L' || hdr->e_ident[3] != 'F') {
        kprint_color("Error: Not a valid ELF executable format.\n", 0x04);
        kfree(file_content); // Jangan lupa kembalikan memori!
        return;
    }

    kprint("Valid ELF detected. Preparing to load...\n");

    // 5. Kalkulasi Alamat Titik Masuk (Entry Point Relatif)
    u32 entry_point = (u32)file_content + hdr->e_entry;
    void (*app_start)() = (void (*)())entry_point;

    kprint_color("Melompat ke User Space (Ring 3)...\n", 0x02);
    
    // 6. EKSEKUSI!
    app_start(); 
    
    kprint_color("\nProgram execution finished.\n", 0x03);

    // 7. Bersihkan memori RAM setelah aplikasi selesai berjalan
    kfree(file_content);
}
#ifndef ELF_H
#define ELF_H

#include "utils.h"

// Struktur Header Utama ELF 32-bit
typedef struct {
    u8  e_ident[16];  // Magic number dan info arsitektur
    u16 e_type;       // Tipe file (executable, dll)
    u16 e_machine;    // Arsitektur (x86)
    u32 e_version;    // Versi ELF
    u32 e_entry;      // TITIK MASUK (Entry Point) program
    u32 e_phoff;      // Offset ke Program Header
    u32 e_shoff;      // Offset ke Section Header
    u32 e_flags;      // Flags spesifik arsitektur
    u16 e_ehsize;     // Ukuran header ini
    u16 e_phentsize;  // Ukuran satu entri Program Header
    u16 e_phnum;      // Jumlah Program Header
    u16 e_shentsize;  // Ukuran satu entri Section Header
    u16 e_shnum;      // Jumlah Section Header
    u16 e_shstrndx;   // Indeks string table
} __attribute__((packed)) elf32_ehdr_t;

void execute_elf(char *filename);

#endif
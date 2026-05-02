#ifndef IDT_H
#define IDT_H

#include "utils.h" // Untuk tipe data u32, u16, u8

// Struktur untuk setiap "Gerbang" Interrupt
typedef struct {
    u16 low_offset;  // 16 bit alamat handler bagian bawah
    u16 sel;         // Kernel Segment Selector
    u8  always0;     // Selalu 0
    u8  flags;       // Flags (Present, DPL, Type)
    u16 high_offset; // 16 bit alamat handler bagian atas
} __attribute__((packed)) idt_gate_t;

// Pointer ke array IDT
typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) idt_register_t;

// Fungsi-fungsi (akan ada di idt.c dan assembly)
void set_idt_gate(int n, u32 handler);
void set_idt();

#endif
#include "gdt.h"

// Struktur entri GDT
typedef struct {
    u16 limit_low;
    u16 base_low;
    u8  base_middle;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed)) gdt_entry_t;

// Struktur pointer GDT
typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) gdt_ptr_t;

// Struktur Task State Segment (TSS)
typedef struct {
    u32 prev_tss; u32 esp0; u32 ss0; u32 esp1; u32 ss1; u32 esp2;
    u32 ss2; u32 cr3; u32 eip; u32 eflags; u32 eax; u32 ecx;
    u32 edx; u32 ebx; u32 esp; u32 ebp; u32 esi; u32 edi;
    u32 es; u32 cs; u32 ss; u32 ds; u32 fs; u32 gs;
    u32 ldt; u16 trap; u16 iomap_base;
} __attribute__((packed)) tss_entry_t;

// 6 Entri: Null, Kernel Code, Kernel Data, User Code, User Data, TSS
gdt_entry_t gdt[6];
gdt_ptr_t gdt_ptr;
tss_entry_t tss_entry;

extern void gdt_flush(u32);
extern void tss_flush();

void gdt_set_gate(int num, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void write_tss(int num, u16 ss0, u32 esp0) {
    u32 base = (u32) &tss_entry;
    u32 limit = base + sizeof(tss_entry_t);

    gdt_set_gate(num, base, limit, 0xE9, 0x00);
    
    // Bersihkan memori TSS
    u8 *tss_ptr = (u8*)&tss_entry;
    for(int i=0; i<sizeof(tss_entry_t); i++) tss_ptr[i] = 0;

    tss_entry.ss0  = ss0;
    tss_entry.esp0 = esp0;
    tss_entry.cs   = 0x0b; // User Code Segment
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13; // User Data Segment
    tss_entry.iomap_base = sizeof(tss_entry_t);
}

void set_kernel_stack(u32 stack) {
    tss_entry.esp0 = stack;
}

void gdt_init() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base  = (u32)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User Code (Ring 3)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User Data (Ring 3)
    
    write_tss(5, 0x10, 0x0); // TSS

    gdt_flush((u32)&gdt_ptr);
    tss_flush();
}
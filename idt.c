#include "idt.h"
#include "ports.h" // Kita butuh port_byte_out

// Array IDT (256 entry)
idt_gate_t idt[256];
idt_register_t idt_reg;

// Import fungsi dari assembly
extern void load_idt(u32 idt_ptr);
extern void keyboard_isr();

// Fungsi untuk mengisi satu gerbang IDT
void set_idt_gate(int n, u32 handler) {
    idt[n].low_offset = (u16)((handler) & 0xFFFF);
    idt[n].sel = 0x08; // Kernel Code Segment Offset
    idt[n].always0 = 0;
    idt[n].flags = 0x8E; // 10001110 (Present, Ring0, Interrupt Gate)
    idt[n].high_offset = (u16)(((handler) >> 16) & 0xFFFF);
}

// Fungsi krusial: Remap PIC (Programmable Interrupt Controller)
// Agar IRQ hardware tidak bentrok dengan Exception CPU
void remap_pic() {
    // ICW1
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    // ICW2 (Remap Offset: Master -> 32 (0x20), Slave -> 40 (0x28))
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    // ICW3
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    // ICW4
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    // Mask (Nyalakan semua interupsi)
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0);
}

void set_idt() {
    // Set pointer IDT
    idt_reg.base = (u32) &idt;
    idt_reg.limit = 256 * sizeof(idt_gate_t) - 1;

    // 1. Remap PIC agar IRQ 0-15 pindah ke 32-47
    remap_pic();

    // 2. Pasang Handler Keyboard (IRQ 1) ke slot 33 (32 + 1)
    set_idt_gate(33, (u32)keyboard_isr);

    // 3. Load IDT ke CPU
    load_idt((u32)&idt_reg);
}
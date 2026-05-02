#include "idt.h"
#include "ports.h"

idt_gate_t idt[256];
idt_register_t idt_reg;

// Import fungsi dari assembly
extern void load_idt(u32 idt_ptr);
extern void keyboard_isr();
extern void isr128(); // <--- Jembatan Syscall Assembly Kita

// Gate untuk Kernel (Ring 0)
void set_idt_gate(int n, u32 handler) {
    idt[n].low_offset = (u16)((handler) & 0xFFFF);
    idt[n].sel = 0x08; 
    idt[n].always0 = 0;
    idt[n].flags = 0x8E; // 10001110 (Present, Ring 0, Interrupt Gate)
    idt[n].high_offset = (u16)(((handler) >> 16) & 0xFFFF);
}

// Gate khusus untuk Aplikasi (User Space / Ring 3)
void set_idt_gate_user(int n, u32 handler) {
    idt[n].low_offset = (u16)((handler) & 0xFFFF);
    idt[n].sel = 0x08; 
    idt[n].always0 = 0;
    idt[n].flags = 0xEE; // 11101110 (Present, Ring 3, Interrupt Gate) <-- KUNCI UTAMA
    idt[n].high_offset = (u16)(((handler) >> 16) & 0xFFFF);
}

void remap_pic() {
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    port_byte_out(0x21, 0xFD); // Hanya izinkan keyboard
    port_byte_out(0xA1, 0xFF);
}

void set_idt() {
    idt_reg.base = (u32) &idt;
    idt_reg.limit = 256 * sizeof(idt_gate_t) - 1;

    remap_pic();

    set_idt_gate(33, (u32)keyboard_isr); // Keyboard
    set_idt_gate_user(128, (u32)isr128); // Syscall 0x80 (Bisa diakses dari luar)

    load_idt((u32)&idt_reg);
}
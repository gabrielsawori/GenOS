#include "timer.h"
#include "ports.h"
#include "screen.h"

// GenOS heartbeat (counting how many times the alarm has rung)
u32 tick = 0;

// External declaration of IDT function
extern void set_idt_gate(int n, u32 handler);

// --- MAIN TIMER HANDLER ---
void timer_handler_c() {
    tick++;
    
    // Scheduler logic will be placed here later!
    // For now, to know the timer works, we print a yellow dot
    // every 1000 ticks (which means 1 second if our frequency is 1000Hz).
    if (tick % 1000 == 0) {
        kprint_color(".", 0x0E); // Yellow dot
    }

    // Acknowledge the interrupt to the PIC (Programmable Interrupt Controller)
    port_byte_out(0x20, 0x20); 
}

// --- ASSEMBLY WRAPPER (To prevent CPU crash during interrupt) ---
void timer_interrupt_wrapper();
__asm__(
    ".global timer_interrupt_wrapper\n"
    "timer_interrupt_wrapper:\n"
    "pusha\n"               // Save current state
    "call timer_handler_c\n"// Call our C handler
    "popa\n"                // Restore state
    "iret\n"                // Return from interrupt
);

// --- PIT INITIALIZATION ---
void timer_init(u32 freq) {
    kprint("=== INITIALIZING PROGRAMMABLE INTERVAL TIMER (PIT) ===\n");
    
    // Attach our alarm to interrupt gate number 32 (IRQ0)
    set_idt_gate(32, (u32)timer_interrupt_wrapper);

    // The formula to program the PIT hardware chip
    u32 divisor = 1193180 / freq; 
    
    port_byte_out(0x43, 0x36); // Command port: Mode 3 (Square Wave Generator)
    port_byte_out(0x40, (u8)(divisor & 0xFF));        // Low byte
    port_byte_out(0x40, (u8)((divisor >> 8) & 0xFF)); // High byte
    
    kprint_color("[SUCCESS] GenOS Heartbeat set to 1000Hz (1 ms/tick)!\n\n", 0x02);
}
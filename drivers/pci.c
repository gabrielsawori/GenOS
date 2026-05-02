#include "pci.h"
#include "ports.h"
#include "screen.h"

// Menyimpan "Koordinat" dan "Kabel" Kartu Jaringan Intel E1000
u32 e1000_mmio_base = 0;
u16 e1000_bus = 0;
u16 e1000_slot = 0;
u16 e1000_func = 0;
u8  e1000_irq = 0; // Variabel baru penampung nomor IRQ

// Pencetak Hex 16-bit
void pci_print_hex(u16 n) {
    char hex[] = "0000";
    char *chars = "0123456789ABCDEF";
    for (int i = 3; i >= 0; i--) {
        hex[i] = chars[n & 0xF];
        n >>= 4;
    }
    kprint("0x"); kprint(hex);
}

// Pencetak Hex 32-bit
void pci_print_hex32(u32 n) {
    char hex[] = "00000000";
    char *chars = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        hex[i] = chars[n & 0xF];
        n >>= 4;
    }
    kprint("0x"); kprint(hex);
}

// Pembaca 16-bit dari Ruang Konfigurasi PCI
u16 pci_read_word(u16 bus, u16 slot, u16 func, u8 offset) {
    u32 address;
    u32 lbus  = (u32)bus;
    u32 lslot = (u32)slot;
    u32 lfunc = (u32)func;
    address = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((u32)0x80000000));
    port_dword_out(0xCF8, address);
    return (u16)((port_dword_in(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

// Pembaca 32-bit dari Ruang Konfigurasi PCI
u32 pci_read_dword(u16 bus, u16 slot, u16 func, u8 offset) {
    u32 address;
    u32 lbus  = (u32)bus;
    u32 lslot = (u32)slot;
    u32 lfunc = (u32)func;
    address = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((u32)0x80000000));
    port_dword_out(0xCF8, address);
    return port_dword_in(0xCFC); 
}

// Radar Pemindai PCI GenOS
void pci_scan() {
    kprint("\n=== MEMULAI PEMINDAIAN HARDWARE PCI ===\n");
    int device_count = 0;
    
    for(u16 bus = 0; bus < 256; bus++) {
        for(u16 slot = 0; slot < 32; slot++) {
            u16 vendor = pci_read_word(bus, slot, 0, 0);
            
            if(vendor != 0xFFFF) {
                u16 device = pci_read_word(bus, slot, 0, 2);
                
                kprint("- Slot Terisi! [Vendor: ");
                pci_print_hex(vendor);
                kprint(" | Model: ");
                pci_print_hex(device);
                kprint("]\n");
                
                // Target Utama: Intel E1000
                if (vendor == 0x8086 && device == 0x100E) {
                    kprint_color("  -> [!] KARTU JARINGAN INTEL E1000 DITEMUKAN!\n", 0x02);
                    
                    // 1. Ambil Alamat MMIO (Register BAR0)
                    u32 bar0 = pci_read_dword(bus, slot, 0, 0x10);
                    e1000_mmio_base = bar0 & 0xFFFFFFF0;
                    e1000_bus = bus;
                    e1000_slot = slot;
                    e1000_func = 0;

                    kprint_color("     [+] ALAMAT MMIO TERTANGKAP: ", 0x03);
                    pci_print_hex32(e1000_mmio_base);
                    kprint("\n");

                    // 2. Ambil Jalur IRQ (Offset 0x3C)
                    u16 intr_data = pci_read_word(bus, slot, 0, 0x3C);
                    e1000_irq = intr_data & 0xFF; // Ambil 8 bit terakhir saja

                    kprint_color("     [+] JALUR IRQ TERTANGKAP: ", 0x03);
                    char irq_str[10];
                    int_to_ascii(e1000_irq, irq_str);
                    kprint(irq_str);
                    kprint("\n");
                }
                device_count++;
            }
        }
    }
    
    kprint("=== TOTAL PERANGKAT: ");
    char count_str[10];
    int_to_ascii(device_count, count_str);
    kprint(count_str);
    kprint(" ===\n\n");
}
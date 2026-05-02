#include "ata.h"
#include "ports.h"

// Tunggu sampai disk siap (BSY bit clear)
void ata_wait_bsy() {
    while(port_byte_in(0x1F7) & 0x80);
}

// Tunggu sampai disk siap mentransfer data (DRQ bit set)
void ata_wait_drq() {
    while(!(port_byte_in(0x1F7) & 0x08));
}

void ata_read_sector(u32 lba, u8 *buffer) {
    ata_wait_bsy();
    
    port_byte_out(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); // Master drive & LBA High
    port_byte_out(0x1F2, 1);                           // Baca 1 sektor (512 byte)
    port_byte_out(0x1F3, (u8) lba);                    // LBA Low
    port_byte_out(0x1F4, (u8)(lba >> 8));              // LBA Mid
    port_byte_out(0x1F5, (u8)(lba >> 16));             // LBA High
    port_byte_out(0x1F7, 0x20);                        // Command: Read Sector

    ata_wait_bsy();
    ata_wait_drq();

    // Ambil data 16-bit (2 byte) sebanyak 256 kali = 512 byte (1 Sektor)
    u16 *ptr = (u16 *) buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = port_word_in(0x1F0);
    }
}

void ata_write_sector(u32 lba, u8 *buffer) {
    ata_wait_bsy();
    
    port_byte_out(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    port_byte_out(0x1F2, 1);
    port_byte_out(0x1F3, (u8) lba);
    port_byte_out(0x1F4, (u8)(lba >> 8));
    port_byte_out(0x1F5, (u8)(lba >> 16));
    port_byte_out(0x1F7, 0x30);                        // Command: Write Sector

    ata_wait_bsy();
    ata_wait_drq();

    // Lempar data 16-bit ke hard disk
    u16 *ptr = (u16 *) buffer;
    for (int i = 0; i < 256; i++) {
        port_word_out(0x1F0, ptr[i]);
    }
}
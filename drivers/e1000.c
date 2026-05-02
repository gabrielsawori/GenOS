#include "e1000.h"
#include "pci.h"
#include "screen.h"
#include "mem.h" 
#include "ports.h" 

extern void set_idt_gate(int n, u32 handler);

#define REG_ICR     0x00C0 
#define REG_IMS     0x00D0 
#define REG_RCTL    0x0100 
#define REG_TCTL    0x0400 
#define REG_RDBAL   0x2800 
#define REG_RDBAH   0x2804 
#define REG_RDLEN   0x2808 
#define REG_RDH     0x2810 
#define REG_RDT     0x2818 
#define REG_TDBAL   0x3800 
#define REG_TDBAH   0x3804 
#define REG_TDLEN   0x3808 
#define REG_TDH     0x3810 
#define REG_TDT     0x3818 

u8 mac_address[6];
struct e1000_rx_desc *rx_ring;
struct e1000_tx_desc *tx_ring;

u16 rx_cur = 0;
u16 tx_cur = 0; // <--- Variabel penunjuk antrean pengirim

u32 mmio_read32(u32 addr) { return (*((volatile u32 *)(addr))); }
void mmio_write32(u32 addr, u32 val) { (*((volatile u32 *)(addr))) = val; }

void print_mac_packet(u8 *mac) {
    char *chars = "0123456789ABCDEF";
    for (int i = 0; i < 6; i++) {
        char hex[3] = {chars[(mac[i] >> 4) & 0xF], chars[mac[i] & 0xF], 0};
        kprint(hex);
        if (i < 5) kprint(":");
    }
}

// ============================================================
// FASE 5: FUNGSI PENGIRIM (TRANSMIT PACKET)
// ============================================================
void e1000_send_packet(u8 *payload, u16 len) {
    // 1. Salin data mentah kita ke dalam buffer TX di RAM
    u8 *tx_buf = (u8 *)tx_ring[tx_cur].addr_low;
    for(int i = 0; i < len; i++) {
        tx_buf[i] = payload[i];
    }

    // 2. Isi Amplop (Descriptor)
    tx_ring[tx_cur].length = len;
    
    // CMD: EOP (End Of Packet - Bit 0) | IFCS (Insert FCS - Bit 1) | RS (Report Status - Bit 3) = 0x0B
    tx_ring[tx_cur].cmd = 0x0B; 
    tx_ring[tx_cur].status = 0; // Bersihkan status, agar hardware yang merubahnya nanti jika selesai

    // 3. Majukan indeks, dan beri tahu Register Hardware (TDT) untuk mengeksekusinya!
    tx_cur = (tx_cur + 1) % E1000_NUM_TX_DESC;
    mmio_write32(e1000_mmio_base + REG_TDT, tx_cur);
}
// ============================================================

void e1000_handler() {
    u32 status = mmio_read32(e1000_mmio_base + REG_ICR);
    
    port_byte_out(0xA0, 0x20); 
    port_byte_out(0x20, 0x20); 

    if (status & 0x80) { 
        while (rx_ring[rx_cur].status & 0x01) {
            
            u8 *packet = (u8 *)rx_ring[rx_cur].addr_low;
            u16 len = rx_ring[rx_cur].length;

            kprint_color("\n[+] PAKET DITERIMA | Tipe: ", 0x0A); 
            
            u16 eth_type = (packet[12] << 8) | packet[13];
            
            if (eth_type == 0x0800) kprint_color("IPv4", 0x0E); 
            else if (eth_type == 0x0806) kprint_color("ARP", 0x0E);
            else if (eth_type == 0x86DD) kprint_color("IPv6", 0x0E);
            else if (eth_type == 0x1337) kprint_color("GENOS-TEST", 0x0D); // Tipe khusus OS kita (Warna Pink)
            else kprint_color("Lainnya", 0x08); 

            kprint(" | Ukuran: ");
            char len_str[10];
            int_to_ascii(len, len_str);
            kprint(len_str);
            kprint(" bytes\n");

            kprint("    -> Dari: "); print_mac_packet(&packet[6]);
            kprint(" | Tujuan: "); print_mac_packet(&packet[0]);
            kprint("\n> ");

            rx_ring[rx_cur].status = 0;
            mmio_write32(e1000_mmio_base + REG_RDT, rx_cur);
            rx_cur = (rx_cur + 1) % E1000_NUM_RX_DESC;
        }
    }
}

void e1000_interrupt_wrapper();
__asm__(
    ".global e1000_interrupt_wrapper\n"
    "e1000_interrupt_wrapper:\n"
    "pusha\n"               
    "call e1000_handler\n"  
    "popa\n"                
    "iret\n"                
);

void e1000_print_mac() {
    kprint_color("[+] MAC Address E1000 Tertangkap: ", 0x03);
    char *chars = "0123456789ABCDEF";
    for (int i = 0; i < 6; i++) {
        char hex[3] = "00";
        hex[0] = chars[(mac_address[i] >> 4) & 0xF];
        hex[1] = chars[mac_address[i] & 0xF];
        kprint(hex);
        if (i < 5) kprint(":");
    }
    kprint("\n");
}

void e1000_init_rx() {
    rx_ring = (struct e1000_rx_desc *) pmm_alloc_block();
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        rx_ring[i].addr_low = (u32) kmalloc(2048); 
        rx_ring[i].addr_high = 0;
        rx_ring[i].status = 0;
    }
    mmio_write32(e1000_mmio_base + REG_RDBAL, (u32)rx_ring);
    mmio_write32(e1000_mmio_base + REG_RDBAH, 0);
    mmio_write32(e1000_mmio_base + REG_RDLEN, E1000_NUM_RX_DESC * 16);
    mmio_write32(e1000_mmio_base + REG_RDH, 0);
    mmio_write32(e1000_mmio_base + REG_RDT, E1000_NUM_RX_DESC - 1);
    mmio_write32(e1000_mmio_base + REG_RCTL, 0x801E); 
}

void e1000_init_tx() {
    tx_ring = (struct e1000_tx_desc *) pmm_alloc_block();
    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        tx_ring[i].addr_low = (u32) kmalloc(2048); // PERBAIKAN: Kini TX juga dapat jatah RAM!
        tx_ring[i].addr_high = 0;
        tx_ring[i].cmd = 0;
        tx_ring[i].status = 1;
    }
    mmio_write32(e1000_mmio_base + REG_TDBAL, (u32)tx_ring);
    mmio_write32(e1000_mmio_base + REG_TDBAH, 0);
    mmio_write32(e1000_mmio_base + REG_TDLEN, E1000_NUM_TX_DESC * 16);
    mmio_write32(e1000_mmio_base + REG_TDH, 0);
    mmio_write32(e1000_mmio_base + REG_TDT, 0);
    
    // TCTL: Mengaktifkan Transmit Buffer
    mmio_write32(e1000_mmio_base + REG_TCTL, 0x0A);
}

void e1000_init() {
    if (e1000_mmio_base == 0) return;
    kprint("=== MEMULAI INISIALISASI INTEL E1000 ===\n");
    u32 ral = mmio_read32(e1000_mmio_base + 0x5400);
    u32 rah = mmio_read32(e1000_mmio_base + 0x5404);
    mac_address[0] = (ral & 0xFF); mac_address[1] = (ral >> 8) & 0xFF;
    mac_address[2] = (ral >> 16) & 0xFF; mac_address[3] = (ral >> 24) & 0xFF;
    mac_address[4] = (rah & 0xFF); mac_address[5] = (rah >> 8) & 0xFF;
    e1000_print_mac();

    e1000_init_rx();
    e1000_init_tx();
    kprint_color("[SUKSES] Intel E1000 Terhubung ke RAM GenOS!\n", 0x02);
    kprint("Menyambungkan IRQ Jaringan ke CPU...\n");
    set_idt_gate(32 + e1000_irq, (u32)e1000_interrupt_wrapper);
    
    mmio_write32(e1000_mmio_base + REG_IMS, 0x1F6DC); 
    mmio_read32(e1000_mmio_base + REG_ICR); 
    kprint_color("[SUKSES] Radar Paket Jaringan Aktif Menunggu...\n\n", 0x02);
}
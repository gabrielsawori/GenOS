#ifndef E1000_H
#define E1000_H

#include "utils.h"

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

struct e1000_rx_desc {
    volatile u32 addr_low;   
    volatile u32 addr_high;  
    volatile u16 length;     
    volatile u16 csum;       
    volatile u8  status;     
    volatile u8  errors;     
    volatile u16 special;
} __attribute__((packed));

struct e1000_tx_desc {
    volatile u32 addr_low;   
    volatile u32 addr_high;  
    volatile u16 length;     
    volatile u8  cso;        
    volatile u8  cmd;        
    volatile u8  status;     
    volatile u8  css;        
    volatile u16 special;
} __attribute__((packed));

// Ekspor variabel dan fungsi agar bisa dipakai di Kernel
extern u8 mac_address[6];

void e1000_init();
void e1000_print_mac();
void e1000_send_packet(u8 *payload, u16 len); // <--- FUNGSI BARU!

#endif
#ifndef NET_H
#define NET_H

#include "utils.h"

u16 htons(u16 v);
u16 ntohs(u16 v);

// 1. Amplop Layer 2 (Ethernet)
struct eth_hdr {
    u8  dest_mac[6];
    u8  src_mac[6];
    u16 eth_type; 
} __attribute__((packed));

// 2. Amplop Layer 3 (ARP)
struct arp_hdr {
    u16 hw_type;      
    u16 proto_type;   
    u8  hw_addr_len;  
    u8  proto_addr_len; 
    u16 opcode;       
    u8  src_mac[6];
    u8  src_ip[4];
    u8  dest_mac[6];
    u8  dest_ip[4];
} __attribute__((packed));

// 3. Amplop Layer 3 (IPv4)
struct ipv4_hdr {
    u8  version_ihl; 
    u8  dscp_ecn;    
    u16 total_length;
    u16 ident;
    u16 flags_frag;
    u8  ttl;
    u8  protocol;    
    u16 checksum;
    u8  src_ip[4];
    u8  dest_ip[4];
} __attribute__((packed));

// 4. Amplop Layer 4 (ICMP PING)
struct icmp_hdr {
    u8  type;        
    u8  code;
    u16 checksum;
    u16 id;
    u16 sequence;
} __attribute__((packed));

// --- STRUKTUR BUKU TELEPON (ARP TABLE) ---
#define ARP_TABLE_SIZE 10
typedef struct {
    u8 ip[4];
    u8 mac[6];
    u8 active; // 1 = Terisi, 0 = Kosong
} arp_entry_t;

void net_init();
void net_handle_packet(u8 *packet, u16 len);
void net_send_ping(); 

#endif
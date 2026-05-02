#include "net.h"
#include "e1000.h"
#include "screen.h"

u8 genos_ip[4] = {10, 0, 2, 15}; 
u8 router_ip[4] = {10, 0, 2, 2}; // IP Tujuan Ping

// BUKU TELEPON KOSONG
arp_entry_t arp_table[ARP_TABLE_SIZE];

u16 htons(u16 v) { return ((v & 0xFF) << 8) | ((v >> 8) & 0xFF); }
u16 ntohs(u16 v) { return htons(v); }

u16 calculate_checksum(void *addr, int count) {
    register u32 sum = 0;
    u16 *ptr = (u16 *)addr;
    while (count > 1) { sum += *ptr++; count -= 2; }
    if (count > 0) sum += *(u8 *)ptr;
    while (sum >> 16) sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}

// --- FUNGSI BELAJAR MAC ADDRESS ---
void arp_learn(u8 *ip, u8 *mac) {
    // Cek apakah sudah ada, jika ya, perbarui
    for(int i=0; i<ARP_TABLE_SIZE; i++) {
        if(arp_table[i].active && arp_table[i].ip[0] == ip[0] && arp_table[i].ip[1] == ip[1] && arp_table[i].ip[2] == ip[2] && arp_table[i].ip[3] == ip[3]) {
            for(int j=0; j<6; j++) arp_table[i].mac[j] = mac[j];
            return;
        }
    }
    // Jika belum ada, masukkan ke slot kosong
    for(int i=0; i<ARP_TABLE_SIZE; i++) {
        if(!arp_table[i].active) {
            arp_table[i].active = 1;
            for(int j=0; j<4; j++) arp_table[i].ip[j] = ip[j];
            for(int j=0; j<6; j++) arp_table[i].mac[j] = mac[j];
            kprint_color("[ARP] Berhasil mencatat MAC Address baru di Buku Telepon GenOS!\n> ", 0x0E); // Kuning
            return;
        }
    }
}

// --- FUNGSI MENCARI DI BUKU TELEPON ---
u8* arp_lookup(u8 *ip) {
    for(int i=0; i<ARP_TABLE_SIZE; i++) {
        if(arp_table[i].active && arp_table[i].ip[0] == ip[0] && arp_table[i].ip[1] == ip[1] && arp_table[i].ip[2] == ip[2] && arp_table[i].ip[3] == ip[3]) {
            return arp_table[i].mac;
        }
    }
    return 0; // Tidak ditemukan
}

void net_init() {
    for(int i=0; i<ARP_TABLE_SIZE; i++) arp_table[i].active = 0; // Bersihkan buku telepon saat booting
    kprint_color("\n=== INISIALISASI NETWORK STACK (L3/L4) ===\n", 0x0B);
    kprint("IP Address GenOS: 10.0.2.15\n");
    kprint("Protokol Aktif: Ethernet, ARP (Dinamis), IPv4, ICMP\n\n");
}

// --- HANDLER ARP ---
void arp_handle(u8 *packet) {
    struct eth_hdr *eth = (struct eth_hdr *)packet;
    struct arp_hdr *arp = (struct arp_hdr *)(packet + sizeof(struct eth_hdr));

    // Setiap ada paket ARP yang masuk, pelajari KTP pengirimnya!
    arp_learn(arp->src_ip, arp->src_mac);

    if (ntohs(arp->opcode) == 1) { // Jika ada yang bertanya
        if (arp->dest_ip[0] == genos_ip[0] && arp->dest_ip[1] == genos_ip[1] &&
            arp->dest_ip[2] == genos_ip[2] && arp->dest_ip[3] == genos_ip[3]) {
            
            u8 reply[64]; 
            struct eth_hdr *reth = (struct eth_hdr *)reply;
            struct arp_hdr *rarp = (struct arp_hdr *)(reply + sizeof(struct eth_hdr));

            for(int i=0; i<6; i++) { reth->dest_mac[i] = eth->src_mac[i]; reth->src_mac[i] = mac_address[i]; }
            reth->eth_type = htons(0x0806);

            rarp->hw_type = htons(1); rarp->proto_type = htons(0x0800);
            rarp->hw_addr_len = 6; rarp->proto_addr_len = 4; rarp->opcode = htons(2);

            for(int i=0; i<6; i++) { rarp->src_mac[i] = mac_address[i]; rarp->dest_mac[i] = arp->src_mac[i]; }
            for(int i=0; i<4; i++) { rarp->src_ip[i] = genos_ip[i]; rarp->dest_ip[i] = arp->src_ip[i]; }
            for(int i=42; i<64; i++) reply[i] = 0;

            e1000_send_packet(reply, 64);
        }
    }
}

void ipv4_handle(u8 *packet, u16 len) {
    struct ipv4_hdr *ip = (struct ipv4_hdr *)(packet + sizeof(struct eth_hdr));
    if (ip->dest_ip[0] == genos_ip[0] && ip->dest_ip[3] == genos_ip[3]) {
        if (ip->protocol == 1) {
            int ip_hdr_len = (ip->version_ihl & 0x0F) * 4;
            struct icmp_hdr *icmp = (struct icmp_hdr *)((u8 *)ip + ip_hdr_len);

            if (icmp->type == 8) kprint_color("\n[ICMP] Menerima Echo Request! Membalas...\n> ", 0x0D);
            else if (icmp->type == 0) {
                kprint_color("\n[ICMP] PING BALASAN (Echo Reply) diterima! GenOS Online!\n> ", 0x02); 
            }
        }
    }
}

// --- PERINTAH MENGIRIM PING ---
void net_send_ping() {
    // 1. CARI ALAMAT DI BUKU TELEPON DULU
    u8 *target_mac = arp_lookup(router_ip);
    
    if (target_mac == 0) {
        // JIKA TIDAK KETEMU, BERTERIAK DULU (ARP REQUEST)!
        kprint_color("[PING] MAC Tujuan tidak diketahui! Mengirimkan ARP Request...\n", 0x04); // Merah
        kprint_color("[PING] Silakan coba ketik 'ping' lagi setelah balasan ARP diterima.\n", 0x04);
        
        u8 req[64]; 
        struct eth_hdr *reth = (struct eth_hdr *)req;
        struct arp_hdr *rarp = (struct arp_hdr *)(req + sizeof(struct eth_hdr));

        for(int i=0; i<6; i++) { reth->dest_mac[i] = 0xFF; reth->src_mac[i] = mac_address[i]; } // Broadcast
        reth->eth_type = htons(0x0806);

        rarp->hw_type = htons(1); rarp->proto_type = htons(0x0800);
        rarp->hw_addr_len = 6; rarp->proto_addr_len = 4; rarp->opcode = htons(1); // Opcode 1 = Tanya

        for(int i=0; i<6; i++) { rarp->src_mac[i] = mac_address[i]; rarp->dest_mac[i] = 0x00; }
        for(int i=0; i<4; i++) { rarp->src_ip[i] = genos_ip[i]; rarp->dest_ip[i] = router_ip[i]; }
        for(int i=42; i<64; i++) req[i] = 0;

        e1000_send_packet(req, 64);
        return; // Batalkan ping sementara
    }

    // 2. JIKA KETEMU, BARU KIRIM PING MENGGUNAKAN MAC YANG DIPELAJARI!
    kprint("Mengirim ICMP Echo Request...\n");
    u8 packet[100];
    struct eth_hdr *eth = (struct eth_hdr *)packet;
    struct ipv4_hdr *ip = (struct ipv4_hdr *)(packet + sizeof(struct eth_hdr));
    struct icmp_hdr *icmp = (struct icmp_hdr *)(packet + sizeof(struct eth_hdr) + sizeof(struct ipv4_hdr));

    // Menggunakan MAC dari hasil belajar!
    for(int i=0; i<6; i++) { eth->dest_mac[i] = target_mac[i]; eth->src_mac[i] = mac_address[i]; }
    eth->eth_type = htons(0x0800); 

    ip->version_ihl = 0x45; ip->dscp_ecn = 0;
    ip->total_length = htons(sizeof(struct ipv4_hdr) + sizeof(struct icmp_hdr) + 32); 
    ip->ident = htons(1); ip->flags_frag = 0; ip->ttl = 64; ip->protocol = 1; 
    for(int i=0; i<4; i++) { ip->src_ip[i] = genos_ip[i]; ip->dest_ip[i] = router_ip[i]; }
    ip->checksum = 0; ip->checksum = calculate_checksum(ip, sizeof(struct ipv4_hdr));

    icmp->type = 8; icmp->code = 0; icmp->id = htons(0x1337); icmp->sequence = htons(1);
    
    u8 *payload = (u8 *)(packet + sizeof(struct eth_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct icmp_hdr));
    for(int i=0; i<32; i++) payload[i] = 'a' + (i % 26); 

    icmp->checksum = 0; icmp->checksum = calculate_checksum(icmp, sizeof(struct icmp_hdr) + 32);

    e1000_send_packet(packet, 74);
}

void net_handle_packet(u8 *packet, u16 len) {
    struct eth_hdr *eth = (struct eth_hdr *)packet;
    u16 eth_type = ntohs(eth->eth_type);

    if (eth_type == 0x0806) arp_handle(packet);
    else if (eth_type == 0x0800) ipv4_handle(packet, len); 
}
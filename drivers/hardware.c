#include "ports.h"
#include "utils.h" // Butuh int_to_ascii

// --- REAL TIME CLOCK (CMOS) ---
#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

u8 get_rtc_register(int reg) {
    port_byte_out(CMOS_ADDRESS, reg);
    return port_byte_in(CMOS_DATA);
}

// Data dari CMOS formatnya BCD (Binary Coded Decimal), harus diubah ke Integer biasa
// Contoh: Jam 09 di BCD itu 0x09, tapi Jam 15 di BCD itu 0x15 (bukan 15 desimal)
u8 bcd_to_binary(u8 bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

void get_time_string(char *buffer) {
    // Baca Detik, Menit, Jam dari register CMOS
    u8 second = bcd_to_binary(get_rtc_register(0x00));
    u8 minute = bcd_to_binary(get_rtc_register(0x02));
    u8 hour   = bcd_to_binary(get_rtc_register(0x04));
    
    // Konversi manual ke String "JJ:MM:DD"
    // Hati-hati: Kita hacky string manipulation di sini
    char tmp[4];
    
    int_to_ascii(hour, tmp);
    if (hour < 10) { buffer[0]='0'; buffer[1]=tmp[0]; } else { buffer[0]=tmp[0]; buffer[1]=tmp[1]; }
    buffer[2] = ':';
    
    int_to_ascii(minute, tmp);
    if (minute < 10) { buffer[3]='0'; buffer[4]=tmp[0]; } else { buffer[3]=tmp[0]; buffer[4]=tmp[1]; }
    buffer[5] = ':';
    
    int_to_ascii(second, tmp);
    if (second < 10) { buffer[6]='0'; buffer[7]=tmp[0]; } else { buffer[6]=tmp[0]; buffer[7]=tmp[1]; }
    buffer[8] = 0;
}

// --- PC SPEAKER (BEEP) ---
void play_sound(u32 nFrequence) {
    u32 Div;
    u8 tmp;

    // Set PIT ke mode speaker
    Div = 1193180 / nFrequence;
    port_byte_out(0x43, 0xb6);
    port_byte_out(0x42, (u8) (Div) );
    port_byte_out(0x42, (u8) (Div >> 8));

    // Nyalakan speaker
    tmp = port_byte_in(0x61);
    if (tmp != (tmp | 3)) {
        port_byte_out(0x61, tmp | 3);
    }
}

void stop_sound() {
    u8 tmp = port_byte_in(0x61) & 0xFC;
    port_byte_out(0x61, tmp);
}

void beep() {
    play_sound(1000); // 1000 Hz
    for(int i=0; i<5000000; i++); // Delay kasar (CPU busy wait)
    stop_sound();
}
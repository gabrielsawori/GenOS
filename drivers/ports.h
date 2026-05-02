#ifndef PORTS_H
#define PORTS_H

#include "utils.h" // <--- INI TAMBAHANNYA AGAR u32 DIKENALI

u8 port_byte_in(u16 port);
void port_byte_out(u16 port, u8 data);

u16 port_word_in(u16 port);
void port_word_out(u16 port, u16 data);

// Fungsi 32-bit untuk PCI
u32 port_dword_in(u16 port);
void port_dword_out(u16 port, u32 data);

#endif
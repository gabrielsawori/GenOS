#ifndef PORTS_H
#define PORTS_H

typedef unsigned char u8;
typedef unsigned short u16;

u8 port_byte_in(u16 port);
void port_byte_out(u16 port, u8 data);

#endif
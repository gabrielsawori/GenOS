#ifndef PCI_H
#define PCI_H

#include "utils.h"

// Fungsi baca 16-bit dan 32-bit
u16 pci_read_word(u16 bus, u16 slot, u16 func, u8 offset);
u32 pci_read_dword(u16 bus, u16 slot, u16 func, u8 offset); // Fungsi Baru!

void pci_scan();

// Variabel Global untuk menyimpan data Intel E1000
extern u32 e1000_mmio_base;
extern u16 e1000_bus;
extern u16 e1000_slot;
extern u16 e1000_func;
extern u8  e1000_irq;

#endif
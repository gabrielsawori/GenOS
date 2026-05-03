#ifndef PAGING_H
#define PAGING_H

#include "utils.h"

// Bendera (Flags) untuk hak akses memori
#define PAGE_PRESENT    0x1  // Bit 0: Apakah memori ini ada di RAM fisik? (1 = Ya)
#define PAGE_RW         0x2  // Bit 1: Boleh ditulis? (1 = Read/Write, 0 = Read Only)
#define PAGE_USER       0x4  // Bit 2: Boleh diakses oleh Ring 3 / Aplikasi? (1 = Ya, 0 = Khusus Kernel)

void paging_init();

#endif
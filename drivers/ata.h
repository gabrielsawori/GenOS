#ifndef ATA_H
#define ATA_H

#include "utils.h"

void ata_read_sector(u32 lba, u8 *buffer);
void ata_write_sector(u32 lba, u8 *buffer);

#endif
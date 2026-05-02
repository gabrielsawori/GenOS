#ifndef MEM_H
#define MEM_H

#include "utils.h"

// 1 Blok PMM standar industri adalah 4KB (4096 bytes)
#define PMM_BLOCK_SIZE 4096 

// --- Physical Memory Manager (PMM) ---
void pmm_init(u32 mem_size);
void pmm_init_region(u32 start, u32 size);
void* pmm_alloc_block();
void pmm_free_block(void* p);

// --- Heap Memory Manager (kmalloc) ---
void mem_init();
void* kmalloc(u32 size);
void kfree(void *ptr);

#endif